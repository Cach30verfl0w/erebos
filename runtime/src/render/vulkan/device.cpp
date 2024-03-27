//   Copyright 2024 Cach30verfl0w
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/**
 * @author Cedric Hammes
 * @since  26/03/2024
 */

#include "erebos/render/vulkan/device.hpp"
#include "rps/runtime/vk/rps_vk_runtime.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace erebos::render::vulkan {
    namespace {
        [[nodiscard]] auto get_device_type(VkPhysicalDeviceType type) noexcept -> std::string_view {
            switch(type) {
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    return "dedicated device";
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    return "virtual device";
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    return "integrated device";
                default:
                    return "device";
            }
        }

        [[nodiscard]] auto find_family_index(VkPhysicalDevice device, VkQueueFlags desired_flags, VkQueueFlags undesired_flags = 0) noexcept
            -> std::optional<uint32_t> {
            uint32_t queue_family_properties_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_properties_count, nullptr);
            std::vector<VkQueueFamilyProperties> properties_list {queue_family_properties_count};
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_properties_count, properties_list.data());

            // Enumerate queue families and acquire queue with desired flags and highest possible queue count
            uint32_t queue_count = 0;
            uint32_t family_index = 0;
            for(auto i = 0; i < properties_list.size(); i++) {
                const auto properties = properties_list.at(i);
                if((properties.queueFlags & desired_flags) == desired_flags &&
                   (undesired_flags != 0 && ((properties.queueFlags & undesired_flags) == 0)) && properties.queueCount > queue_count) {
                    queue_count = properties.queueCount;
                    family_index = i;
                }
            }

            // Return index of queue family if queue was found
            if(queue_count > 0) {
                return family_index;
            }
            return {};
        }

        auto rps_realloc([[maybe_unused]] void* user_context,
                         void* pointer,
                         [[maybe_unused]] size_t old_size,
                         size_t new_size,
                         size_t alignment) noexcept -> void* {
            return ::mi_realloc_aligned(pointer, new_size, alignment);
        }

        auto rps_alloc([[maybe_unused]] void* pContext, size_t size, size_t alignment) noexcept -> void* {
            return ::mi_malloc_aligned(size, alignment);
        }

        auto rps_free([[maybe_unused]] void* user_context, void* pointer) noexcept -> void {
            ::mi_free(pointer);
        }

        auto rps_printf([[maybe_unused]] void* user_context, const char* format, ...) noexcept -> void {
            va_list args;
            va_start(args, format);
            ::vprintf(format, args);
            va_end(args);
        }

        auto rps_vprintf([[maybe_unused]] void* user_context, const char* format, va_list args) noexcept -> void {
            ::vprintf(format, args);
        }
    }// namespace

    Device::Device(const VulkanContext& vulkan_context, VkPhysicalDevice physical_device)
        : _context(&vulkan_context)
        , _physical_device(physical_device)
        , _device_handle()
        , _rps_device()
        , _allocator()
        , _queues() {

        // Get queue indices
        // clang-format off
        const auto direct_queue_index = find_family_index(_physical_device, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT
                                                                | VK_QUEUE_TRANSFER_BIT).value_or(0);

        // Acquire index of compute queue family or use direct queue family index as backup
        auto compute_queue_index = find_family_index(_physical_device, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT)
                .value_or(find_family_index(_physical_device, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT)
                .value_or(find_family_index(_physical_device, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT)
                .value_or(direct_queue_index)));

        // Acquire index of transfer queue family or use direct queue family index as backup
        auto transfer_queue_index = find_family_index(_physical_device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)
                .value_or(find_family_index(_physical_device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT)
                .value_or(find_family_index(_physical_device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_COMPUTE_BIT)
                .value_or(direct_queue_index)));
        // clang-format on

        // Create default queue create infos with direct queue index (with one queue in the families)
        constexpr auto queue_properties = 1.0f;
        VkDeviceQueueCreateInfo direct_queue_create_info {};
        direct_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        direct_queue_create_info.queueFamilyIndex = direct_queue_index;
        direct_queue_create_info.pQueuePriorities = &queue_properties;
        direct_queue_create_info.queueCount = 1;
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos {direct_queue_create_info};

        // Add separate queue create info for compute queue family if family is not direct queue family (with one queue in the family)
        if(compute_queue_index != direct_queue_index) {
            VkDeviceQueueCreateInfo compute_queue_create_info {};
            compute_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            compute_queue_create_info.queueFamilyIndex = compute_queue_index;
            compute_queue_create_info.pQueuePriorities = &queue_properties;
            compute_queue_create_info.queueCount = 1;
            queue_create_infos.push_back(compute_queue_create_info);
        }

        // Add separate queue create info for transfer queue family if family is not direct queue family (with one queue in the family)
        if(transfer_queue_index != direct_queue_index) {
            VkDeviceQueueCreateInfo transfer_queue_create_info {};
            transfer_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            transfer_queue_create_info.queueFamilyIndex = transfer_queue_index;
            transfer_queue_create_info.pQueuePriorities = &queue_properties;
            transfer_queue_create_info.queueCount = 1;
            queue_create_infos.push_back(transfer_queue_create_info);
        }

        // clang-format off
        constexpr std::array<const char*, 2> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
        };
        // clang-format on

        // Configure Vulkan 1.3 device features
        VkPhysicalDeviceVulkan13Features vulkan13_features {};
        vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vulkan13_features.dynamicRendering = true;

        // Configure Vulkan 1.2 device features
        VkPhysicalDeviceVulkan12Features vulkan12_features {};
        vulkan13_features.pNext = &vulkan13_features;
        vulkan12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vulkan12_features.timelineSemaphore = true;

        // Configure device features
        VkPhysicalDeviceFeatures2 features {};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = &vulkan12_features;

        VkPhysicalDeviceProperties device_properties {};
        ::vkGetPhysicalDeviceProperties(_physical_device, &device_properties);

        // Create device
        VkDeviceCreateInfo device_create_info {};
        device_create_info.pNext = &features;
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.queueCreateInfoCount = queue_create_infos.size();
        device_create_info.enabledExtensionCount = device_extensions.size();
        device_create_info.ppEnabledExtensionNames = device_extensions.data();
        if(const auto error = ::vkCreateDevice(_physical_device, &device_create_info, nullptr, &_device_handle); error != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create device: {}", vk_strerror(error))};
        }
        ::volkLoadDevice(_device_handle);
        SPDLOG_INFO("Successfully created {} '{}' (Driver v{}.{}.{})",
                    get_device_type(device_properties.deviceType),
                    device_properties.deviceName,
                    VK_API_VERSION_MAJOR(device_properties.driverVersion),
                    VK_API_VERSION_MINOR(device_properties.driverVersion),
                    VK_API_VERSION_PATCH(device_properties.driverVersion));

        // Initialize queues and print out information about these queues
        _queues.emplace_back(_device_handle, direct_queue_index, 0);
        _queues.emplace_back(_device_handle, compute_queue_index, 0);
        _queues.emplace_back(_device_handle, transfer_queue_index, 0);
        SPDLOG_INFO("Initializes queues for '{}' -> Direct Queue ({}) = {}, Compute Queue ({}) = {}, Transfer Queue ({}) = {}",
                    device_properties.deviceName,
                    direct_queue_index,
                    fmt::ptr(*_queues[0]),
                    compute_queue_index,
                    fmt::ptr(*_queues[1]),
                    transfer_queue_index,
                    fmt::ptr(*_queues[2]));

        // Initialize Vulkan functions struct for VMA
        VmaVulkanFunctions vma_vulkan_functions {};
        vma_vulkan_functions.vkAllocateMemory = vkAllocateMemory;
        vma_vulkan_functions.vkBindBufferMemory = vkBindBufferMemory;
        vma_vulkan_functions.vkBindImageMemory = vkBindImageMemory;
        vma_vulkan_functions.vkCreateBuffer = vkCreateBuffer;
        vma_vulkan_functions.vkCreateImage = vkCreateImage;
        vma_vulkan_functions.vkDestroyBuffer = vkDestroyBuffer;
        vma_vulkan_functions.vkDestroyImage = vkDestroyImage;
        vma_vulkan_functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vma_vulkan_functions.vkFreeMemory = vkFreeMemory;
        vma_vulkan_functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vma_vulkan_functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vma_vulkan_functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vma_vulkan_functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vma_vulkan_functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vma_vulkan_functions.vkMapMemory = vkMapMemory;
        vma_vulkan_functions.vkUnmapMemory = vkUnmapMemory;
        vma_vulkan_functions.vkCmdCopyBuffer = vkCmdCopyBuffer;
        vma_vulkan_functions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
        vma_vulkan_functions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
        vma_vulkan_functions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
        vma_vulkan_functions.vkBindImageMemory2KHR = vkBindImageMemory2;
        vma_vulkan_functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;

        // Initialize VMA allocator for this device
        VmaAllocatorCreateInfo allocator_create_info {};
        allocator_create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        allocator_create_info.vulkanApiVersion = _context->get_api_version();
        allocator_create_info.physicalDevice = _physical_device;
        allocator_create_info.device = _device_handle;
        allocator_create_info.instance = **_context;
        allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;
        if(const auto err = ::vmaCreateAllocator(&allocator_create_info, &_allocator); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create memory allocator: {}", vk_strerror(err))};
        }

        // Initialize Vulkan functions struct for RPS
        RpsVKFunctions rps_vulkan_functions {};
        rps_vulkan_functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        rps_vulkan_functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        rps_vulkan_functions.vkCreateImage = vkCreateImage;
        rps_vulkan_functions.vkDestroyImage = vkDestroyImage;
        rps_vulkan_functions.vkBindImageMemory = vkBindImageMemory;
        rps_vulkan_functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        rps_vulkan_functions.vkCreateBuffer = vkCreateBuffer;
        rps_vulkan_functions.vkDestroyBuffer = vkDestroyBuffer;
        rps_vulkan_functions.vkBindBufferMemory = vkBindBufferMemory;
        rps_vulkan_functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        rps_vulkan_functions.vkCreateFramebuffer = vkCreateFramebuffer;
        rps_vulkan_functions.vkDestroyFramebuffer = vkDestroyFramebuffer;
        rps_vulkan_functions.vkCreateRenderPass = vkCreateRenderPass;
        rps_vulkan_functions.vkDestroyRenderPass = vkDestroyRenderPass;
        rps_vulkan_functions.vkCreateBufferView = vkCreateBufferView;
        rps_vulkan_functions.vkDestroyBufferView = vkDestroyBufferView;
        rps_vulkan_functions.vkCreateImageView = vkCreateImageView;
        rps_vulkan_functions.vkDestroyImageView = vkDestroyImageView;
        rps_vulkan_functions.vkAllocateMemory = vkAllocateMemory;
        rps_vulkan_functions.vkFreeMemory = vkFreeMemory;
        rps_vulkan_functions.vkCmdBeginRenderPass = vkCmdBeginRenderPass;
        rps_vulkan_functions.vkCmdEndRenderPass = vkCmdEndRenderPass;
        rps_vulkan_functions.vkCmdSetViewport = vkCmdSetViewport;
        rps_vulkan_functions.vkCmdSetScissor = vkCmdSetScissor;
        rps_vulkan_functions.vkCmdPipelineBarrier = vkCmdPipelineBarrier;
        rps_vulkan_functions.vkCmdClearColorImage = vkCmdClearColorImage;
        rps_vulkan_functions.vkCmdClearDepthStencilImage = vkCmdClearDepthStencilImage;
        rps_vulkan_functions.vkCmdCopyImage = vkCmdCopyImage;
        rps_vulkan_functions.vkCmdCopyBuffer = vkCmdCopyBuffer;
        rps_vulkan_functions.vkCmdCopyImageToBuffer = vkCmdCopyImageToBuffer;
        rps_vulkan_functions.vkCmdCopyBufferToImage = vkCmdCopyBufferToImage;
        rps_vulkan_functions.vkCmdResolveImage = vkCmdResolveImage;
        rps_vulkan_functions.vkCmdBeginRendering = vkCmdBeginRendering;
        rps_vulkan_functions.vkCmdEndRendering = vkCmdEndRendering;

        // Initialize RPS runtime device for this device
        RpsDeviceCreateInfo rps_device_create_info {};
        rps_device_create_info.allocator.pfnAlloc = rps_alloc;
        rps_device_create_info.allocator.pfnRealloc = rps_realloc;
        rps_device_create_info.allocator.pfnFree = rps_free;
        rps_device_create_info.printer.pfnVPrintf = rps_vprintf;
        rps_device_create_info.printer.pfnPrintf = rps_printf;

        RpsRuntimeDeviceCreateInfo rps_runtime_device_create_info {};
        RpsVKRuntimeDeviceCreateInfo vk_runtime_device_create_info {};
        vk_runtime_device_create_info.flags = RPS_VK_RUNTIME_FLAG_DONT_FLIP_VIEWPORT;
        vk_runtime_device_create_info.pDeviceCreateInfo = &rps_device_create_info;
        vk_runtime_device_create_info.pRuntimeCreateInfo = &rps_runtime_device_create_info;
        vk_runtime_device_create_info.hVkDevice = _device_handle;
        vk_runtime_device_create_info.hVkPhysicalDevice = _physical_device;
        vk_runtime_device_create_info.pVkFunctions = &rps_vulkan_functions;
        if(const auto error = rpsVKRuntimeDeviceCreate(&vk_runtime_device_create_info, &_rps_device); error < 0) {
            throw std::runtime_error {fmt::format("Unable to initialize RPS device: {}", rpsResultGetName(error))};
        }
    }

    Device::Device(Device&& other) noexcept
        : _context(other._context)
        , _physical_device(other._physical_device)
        , _device_handle(other._device_handle)
        , _rps_device(other._rps_device)
        , _allocator(other._allocator)
        , _queues(std::move(other._queues)) {
        _device_handle = nullptr;
        _rps_device = nullptr;
        _allocator = nullptr;
    }

    Device::~Device() noexcept {
        if(_allocator != nullptr) {
            ::vmaDestroyAllocator(_allocator);
            _allocator = nullptr;
        }

        if(_rps_device != nullptr) {
            ::rpsDeviceDestroy(_rps_device);
            _rps_device = nullptr;
        }

        if(_device_handle != nullptr) {
            ::vkDestroyDevice(_device_handle, nullptr);
            _device_handle = nullptr;
        }
    }

    auto Device::operator=(Device&& other) noexcept -> Device& {
        _context = other._context;
        _physical_device = other._physical_device;
        _device_handle = other._device_handle;
        _rps_device = other._rps_device;
        _allocator = other._allocator;
        _queues = std::move(other._queues);
        _device_handle = nullptr;
        _rps_device = nullptr;
        _allocator = nullptr;
        return *this;
    }
}// namespace erebos::render::vulkan