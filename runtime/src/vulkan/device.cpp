//  Copyright 2024 Cach30verfl0w
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

/**
 * @author Cedric Hammes
 * @since  14/03/2024
 */

#include "erebos/vulkan/device.hpp"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace erebos::vulkan {
    namespace {
        [[nodiscard]] auto find_queue_family_index(VkPhysicalDevice physical_device,
                                                   VkQueueFlags desired_flags,
                                                   VkQueueFlags undesired_flags = 0) noexcept -> kstd::Option<uint32_t> {
            uint32_t queue_family_properties_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, nullptr);
            std::vector<VkQueueFamilyProperties> properties_list {queue_family_properties_count};
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, properties_list.data());

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
                return {family_index};
            }
            return {};
        }

        [[nodiscard]] auto find_queue_family_indices(VkPhysicalDevice device) noexcept -> std::tuple<uint32_t, uint32_t, uint32_t> {
            auto direct_index = find_queue_family_index(device, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
            // clang-format off
            auto compute_queue_index = find_queue_family_index(device, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT)
                    .get_or(find_queue_family_index(device, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT)
                    .get_or(find_queue_family_index(device, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT)
                    .get_or(direct_index)));

            auto transfer_queue_index = find_queue_family_index(device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)
                    .get_or(find_queue_family_index(device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT)
                    .get_or(find_queue_family_index(device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_COMPUTE_BIT)
                    .get_or(direct_index)));
            // clang-format on
            return {direct_index, compute_queue_index, transfer_queue_index};
        }

        auto get_device_local_heap(VkPhysicalDevice device_handle) -> uint32_t {
            VkPhysicalDeviceMemoryProperties memory_properties {};
            vkGetPhysicalDeviceMemoryProperties(device_handle, &memory_properties);

            uint32_t local_heap_size = 0;
            for(uint32_t i = 0; i < memory_properties.memoryHeapCount; i++) {
                const auto heap = memory_properties.memoryHeaps[i];
                if((heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                    continue;

                local_heap_size += heap.size;
            }
            return local_heap_size;
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

        constexpr auto compare_by_local_heap = [](const auto& left, const auto& right) -> bool {
            return get_device_local_heap(left) < get_device_local_heap(right);
        };
    }// namespace

    /**
     * This constructor creates a virtual device handle by the specified physical device
     * handle.
     *
     * @param physical_device_handle The handle to the physical device
     * @author                       Cedric Hammes
     * @since                        14/03/2024
     */
    Device::Device(const VulkanContext& context, VkPhysicalDevice physical_device_handle)
        : _phy_device {physical_device_handle}
        , _device {}
        , _queues {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE}
        , _runtime_device {}
        , _allocator {} {
        constexpr std::array<const char*, 2> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                                  VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME};

        // Get properties

        VkPhysicalDeviceProperties device_properties {};
        ::vkGetPhysicalDeviceProperties(_phy_device, &device_properties);

        // Enable dynamic rendering for device
        VkPhysicalDeviceVulkan13Features vulkan13_features {};
        vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vulkan13_features.dynamicRendering = VK_TRUE;

        VkPhysicalDeviceVulkan12Features vulkan12_features {};
        vulkan13_features.pNext = &vulkan13_features;
        vulkan12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vulkan12_features.timelineSemaphore = true;

        VkPhysicalDeviceFeatures2 features {};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = &vulkan12_features;

        // Create device queues, Credits: https://github.com/ProjectKML/greek/blob/main/crates/greek_render/src/backend/device.rs#L601-L619
        constexpr auto queue_properties = 1.0f;
        const auto [direct_queue_index, compute_queue_index, transfer_queue_index] = find_queue_family_indices(_phy_device);
        VkDeviceQueueCreateInfo direct_queue_create_info {};
        direct_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        direct_queue_create_info.queueFamilyIndex = direct_queue_index;
        direct_queue_create_info.pQueuePriorities = &queue_properties;
        direct_queue_create_info.queueCount = 1;
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos {3, direct_queue_create_info};

        if(compute_queue_index != direct_queue_index) {
            VkDeviceQueueCreateInfo compute_queue_create_info {};
            compute_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            compute_queue_create_info.queueFamilyIndex = compute_queue_index;
            compute_queue_create_info.pQueuePriorities = &queue_properties;
            compute_queue_create_info.queueCount = 1;
            queue_create_infos[1] = compute_queue_create_info;
        }

        if(transfer_queue_index != direct_queue_index) {
            VkDeviceQueueCreateInfo transfer_queue_create_info {};
            transfer_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            transfer_queue_create_info.queueFamilyIndex = transfer_queue_index;
            transfer_queue_create_info.pQueuePriorities = &queue_properties;
            transfer_queue_create_info.queueCount = 1;
            queue_create_infos[2] = transfer_queue_create_info;
        }
        SPDLOG_INFO("Create queues for device (Direct Queue Index = {}, Transfer Queue Family = {}, Compute Queue Family = {})",
                    direct_queue_index,
                    transfer_queue_index,
                    compute_queue_index);

        // Create device itself
        SPDLOG_INFO("Creating device '{}' (Driver Version: {}.{}.{})",
                    device_properties.deviceName,
                    VK_API_VERSION_MAJOR(device_properties.driverVersion),
                    VK_API_VERSION_MINOR(device_properties.driverVersion),
                    VK_API_VERSION_PATCH(device_properties.driverVersion));

        VkDeviceCreateInfo device_create_info {};
        device_create_info.pNext = &features;
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.queueCreateInfoCount = 3;
        device_create_info.enabledLayerCount = 0;
        device_create_info.enabledExtensionCount = device_extensions.size();
        device_create_info.ppEnabledExtensionNames = device_extensions.data();
        if(const auto err = ::vkCreateDevice(_phy_device, &device_create_info, nullptr, &_device); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create device: {}", vk_strerror(err))};
        }
        ::volkLoadDevice(_device);

        // Initialize queues

        // Create RenderPipelineShaders
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
        vk_runtime_device_create_info.hVkDevice = _device;
        vk_runtime_device_create_info.hVkPhysicalDevice = _phy_device;
        vk_runtime_device_create_info.pVkFunctions = &rps_vulkan_functions;
        if(const auto error = rpsVKRuntimeDeviceCreate(&vk_runtime_device_create_info, &_runtime_device); error < 0) {
            throw std::runtime_error {fmt::format("Unable to initialize RPS device: {}", rpsResultGetName(error))};
        }

        // Create memory allocator
        uint32_t instance_version {};
        if(const auto err = ::vkEnumerateInstanceVersion(&instance_version); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to get instance version: {}", vk_strerror(err))};
        }

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

        VmaAllocatorCreateInfo allocator_create_info {};
        allocator_create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        allocator_create_info.vulkanApiVersion = instance_version;
        allocator_create_info.physicalDevice = _phy_device;
        allocator_create_info.device = _device;
        allocator_create_info.instance = *context;
        allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;
        if(const auto err = ::vmaCreateAllocator(&allocator_create_info, &_allocator); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create memory allocator: {}", vk_strerror(err))};
        }
    }

    Device::Device(Device&& other) noexcept
        : _phy_device {other._phy_device}
        , _device {other._device}
        , _queues {other._queues}
        , _allocator {other._allocator}
        , _runtime_device {other._runtime_device} {
        other._phy_device = nullptr;
        other._device = nullptr;
        other._allocator = nullptr;
        other._runtime_device = nullptr;
    }

    /**
     * This destructor destroys the device, when the device was not destroyed
     * already.
     *
     * @author Cedric Hammes
     * @since  14/03/2024
     */
    Device::~Device() noexcept {
        if(_runtime_device != nullptr) {
            ::rpsDeviceDestroy(_runtime_device);
            _runtime_device = nullptr;
        }

        if(_allocator != nullptr) {
            ::vmaDestroyAllocator(_allocator);
            _allocator = nullptr;
        }

        if(_device != nullptr) {
            ::vkDestroyDevice(_device, nullptr);// TODO: SIGABORT WTF?
            _device = nullptr;
        }
    }

    auto Device::operator=(Device&& other) noexcept -> Device& {
        _phy_device = other._phy_device;
        _device = other._device;
        _queues = other._queues;
        _allocator = other._allocator;
        _runtime_device = other._runtime_device;
        other._phy_device = nullptr;
        other._device = nullptr;
        other._allocator = nullptr;
        other._runtime_device = nullptr;
        return *this;
    }

    /**
     * This function enumerates all devices and returns that device with the biggest local
     * heap.
     *
     * @param context The context of the Vulkan application
     * @return        The found device or an error
     * @author        Cedric Hammes
     * @since         14/03/2024
     */
    [[nodiscard]] auto find_device(const VulkanContext& context) noexcept -> kstd::Result<Device> {
        uint32_t device_count = 0;
        if(const auto err = vkEnumeratePhysicalDevices(*context, &device_count, nullptr); err != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to get physical device count: {}", vk_strerror(err))};
        }
        SPDLOG_INFO("Found {} devices in total", device_count);

        std::vector<VkPhysicalDevice> devices {device_count};
        if(const auto err = vkEnumeratePhysicalDevices(*context, &device_count, devices.data()); err != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to get physical devices: {}", vk_strerror(err))};
        }

        return {{context, devices.at(0)}};
    }
}// namespace erebos::vulkan