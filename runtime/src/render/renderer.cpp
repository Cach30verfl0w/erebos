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
 * @since  24/03/2024
 */

#include "erebos/render/renderer.hpp"

RPS_DECLARE_RPSL_ENTRY(runtime, main)

namespace erebos::render {

    Renderer::Renderer(const vulkan::VulkanContext& context, const vulkan::Device& device)
        : _render_graph_handle {}
        , _swapchain {context, device}
        , _vulkan_context {&context}
        , _vulkan_device {&device}
        , _timeline_semaphore {}
        , _rendering_done_semaphore {}
        , _image_acquired_semaphore {} {
        uint32_t queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device.get_physical_device(), &queue_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties_list {queue_count};
        vkGetPhysicalDeviceQueueFamilyProperties(device.get_physical_device(), &queue_count, queue_family_properties_list.data());

        // Collect flags
        RpsQueueFlags queue_flags = RpsQueueFlagBits::RPS_QUEUE_FLAG_NONE;
        for(auto queue_family_properties : queue_family_properties_list) {
            const auto flags = queue_family_properties.queueFlags;

            if(is_flag_set<VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT>(flags)) {
                queue_flags |= RpsQueueFlagBits::RPS_QUEUE_FLAG_GRAPHICS;
            }

            if(is_flag_set<VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT>(flags)) {
                queue_flags |= RpsQueueFlagBits::RPS_QUEUE_FLAG_COMPUTE;
            }

            if(is_flag_set<VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT>(flags)) {
                queue_flags |= RpsQueueFlagBits::RPS_QUEUE_FLAG_COPY;
            }
        }

        // Create render graph itself
        RpsRenderGraphCreateInfo render_graph_create_info {};
        render_graph_create_info.scheduleInfo.pQueueInfos = &queue_flags;
        render_graph_create_info.scheduleInfo.numQueues = queue_count;
        render_graph_create_info.scheduleInfo.scheduleFlags = RPS_SCHEDULE_DEFAULT;
        render_graph_create_info.mainEntryCreateInfo.hRpslEntryPoint = RPS_ENTRY_REF(runtime, main);
        if(const auto error = rpsRenderGraphCreate(device.get_rps_device(), &render_graph_create_info, &_render_graph_handle); error < 0) {
            throw std::runtime_error {fmt::format("Unable to create renderer: {}", rpsResultGetName(error))};
        }

        // Create semaphores
        VkSemaphoreTypeCreateInfo semaphore_type_create_info {};
        semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphore_type_create_info.initialValue = 0;

        {
            VkSemaphoreCreateInfo semaphore_create_info {};
            semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphore_create_info.pNext = &semaphore_type_create_info;
            if(const auto error = vkCreateSemaphore(**_vulkan_device, &semaphore_create_info, nullptr, &_timeline_semaphore)) {
                throw std::runtime_error {fmt::format("Unable to create renderer: {}", vk_strerror(error))};
            }
        }

        VkSemaphoreCreateInfo semaphore_create_info {};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if(const auto error = vkCreateSemaphore(**_vulkan_device, &semaphore_create_info, nullptr, &_rendering_done_semaphore)) {
            throw std::runtime_error {fmt::format("Unable to create renderer: {}", vk_strerror(error))};
        }

        if(const auto error = vkCreateSemaphore(**_vulkan_device, &semaphore_create_info, nullptr, &_image_acquired_semaphore)) {
            throw std::runtime_error {fmt::format("Unable to create renderer: {}", vk_strerror(error))};
        }
    }

    Renderer::Renderer(Renderer&& other) noexcept
        : _render_graph_handle {other._render_graph_handle}
        , _swapchain {std::move(other._swapchain)}
        , _vulkan_context {other._vulkan_context}
        , _vulkan_device {other._vulkan_device}
        , _timeline_semaphore {other._timeline_semaphore}
        , _rendering_done_semaphore {other._rendering_done_semaphore}
        , _image_acquired_semaphore {other._image_acquired_semaphore} {
        other._render_graph_handle = nullptr;
        other._vulkan_device = nullptr;
        other._timeline_semaphore = nullptr;
        other._rendering_done_semaphore = nullptr;
        other._image_acquired_semaphore = nullptr;
    }

    Renderer::~Renderer() noexcept {
        if(_render_graph_handle != nullptr) {
            rpsRenderGraphDestroy(_render_graph_handle);
            _render_graph_handle = nullptr;
        }

        if(_timeline_semaphore != nullptr) {
            vkDestroySemaphore(**_vulkan_device, _timeline_semaphore, nullptr);
            _timeline_semaphore = nullptr;
        }

        if(_rendering_done_semaphore != nullptr) {
            vkDestroySemaphore(**_vulkan_device, _rendering_done_semaphore, nullptr);
            _rendering_done_semaphore = nullptr;
        }

        if(_image_acquired_semaphore != nullptr) {
            vkDestroySemaphore(**_vulkan_device, _image_acquired_semaphore, nullptr);
            _image_acquired_semaphore = nullptr;
        }
    }

    auto Renderer::render() const noexcept -> kstd::Result<void> {
        RpsRenderGraphBatchLayout batch_layout {};
        if(const auto error = rpsRenderGraphGetBatchLayout(_render_graph_handle, &batch_layout); error < 0) {
            return kstd::Error {fmt::format("Unable to render with renderer: {}", rpsResultGetName(error))};
        }

        for(auto i = 0; i < batch_layout.numCmdBatches; i++) {
            RpsCommandBatch batch = batch_layout.pCmdBatches[i];

            // Record render graph commands
            const auto command_pool = kstd::try_construct<vulkan::CommandPool>(*_vulkan_device, batch.queueIndex);
            if(!command_pool) {
                return kstd::Error {command_pool.get_error()};
            }

            const auto command_buffer = std::move(command_pool->allocate(1)->at(0));
            if(const auto error = command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT); !error) {
                return kstd::Error {error.get_error()};
            }

            RpsRenderGraphRecordCommandInfo record_command_info {};
            record_command_info.hCmdBuffer = rpsVKCommandBufferToHandle(*command_buffer);
            record_command_info.cmdBeginIndex = batch.cmdBegin;
            record_command_info.numCmds = batch.cmdBegin;
            record_command_info.frameIndex = _swapchain.current_image_index();
            if(const auto error = rpsRenderGraphRecordCommands(_render_graph_handle, &record_command_info); error < 0) {
                return kstd::Error {fmt::format("Unable to render with renderer: {}", rpsResultGetName(error))};
            }

            if(const auto error = command_buffer.end(); !error) {
                return kstd::Error {error.get_error()};
            }

            // Create values
            const auto wait_fences_count = batch.numWaitFences;
            std::vector<VkSemaphore> wait_semaphores = std::vector(wait_fences_count, _timeline_semaphore);
            std::vector<uint64_t> wait_semaphore_values = std::vector(wait_fences_count, static_cast<uint64_t>(0));
            for(auto j = 0; j < wait_fences_count; j++) {
                wait_semaphore_values[i] = *(batch_layout.pWaitFenceIndices + batch.waitFencesBegin + i);
            }

            std::vector<VkSemaphore> signal_semaphores = std::vector(wait_fences_count, _timeline_semaphore);
            std::vector<VkPipelineStageFlags> wait_dst_stage_masks =
                std::vector(wait_fences_count, static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT));
            std::vector<uint64_t> signal_semaphore_values = std::vector(wait_fences_count, static_cast<uint64_t>(batch.signalFenceIndex));
            auto fence = vulkan::sync::Fence {*_vulkan_device};

            if(i == 0) {
                wait_semaphores.push_back(_image_acquired_semaphore);
                wait_semaphore_values.push_back(0);
                wait_dst_stage_masks.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
            }
            else {
                signal_semaphores.push_back(_rendering_done_semaphore);
                signal_semaphore_values.push_back(0);
            }

            // Submit and wait
            VkTimelineSemaphoreSubmitInfo semaphore_submit_info {};
            semaphore_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
            semaphore_submit_info.waitSemaphoreValueCount = wait_semaphore_values.size();
            semaphore_submit_info.pWaitSemaphoreValues = wait_semaphore_values.data();
            semaphore_submit_info.signalSemaphoreValueCount = signal_semaphore_values.size();
            semaphore_submit_info.pSignalSemaphoreValues = signal_semaphore_values.data();

            const auto raw_command_buffer = *command_buffer;
            VkSubmitInfo submit_info {};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pNext = &semaphore_submit_info;
            submit_info.waitSemaphoreCount = wait_semaphores.size();
            submit_info.pWaitSemaphores = wait_semaphores.data();
            submit_info.signalSemaphoreCount = signal_semaphores.size();
            submit_info.pSignalSemaphores = signal_semaphores.data();
            submit_info.pWaitDstStageMask = wait_dst_stage_masks.data();
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &raw_command_buffer;

            VkQueue queue {};
            vkGetDeviceQueue(**_vulkan_device, 0, batch.queueIndex, &queue);
            if(const auto error = vkQueueSubmit(queue, 1, &submit_info, *fence); error != VK_SUCCESS) {
                return kstd::Error {fmt::format("Unable to render with renderer: {}", vk_strerror(error))};
            }

            if(const auto error = fence.wait_for(); !error) {
                return kstd::Error {error.get_error()};
            }
        }

        const auto raw_swapchain = *_swapchain;
        const auto current_image_index = _swapchain.current_image_index();
        VkPresentInfoKHR present_info {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &_rendering_done_semaphore;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &raw_swapchain;
        present_info.pImageIndices = &current_image_index;
        if(const auto error = vkQueuePresentKHR(_vulkan_device->get_graphics_queue(), &present_info); error != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to render with renderer: {}", vk_strerror(error))};
        }
        return {};
    }

    auto Renderer::update() const noexcept -> kstd::Result<void> {
        std::vector<RpsRuntimeResource> image_handles {_swapchain.images().size()};
        for(auto i = 0; i < image_handles.size(); i++) {
            image_handles[i] = rpsVKImageToHandle(_swapchain.images()[i]);
        }

        const auto window_surface = SDL_GetWindowSurface(**_vulkan_context->get_window());
        auto resource_desc = RpsResourceDesc {};
        resource_desc.image.format = rpsFormatFromVK(VK_FORMAT_B8G8R8A8_UNORM);
        resource_desc.image.width = window_surface->w;
        resource_desc.image.height = window_surface->h;
        resource_desc.image.arrayLayers = 1;
        resource_desc.image.depth = 1;
        resource_desc.image.mipLevels = 1;
        resource_desc.image.sampleCount = 1;
        resource_desc.type = RpsResourceType::RPS_RESOURCE_TYPE_IMAGE_2D;
        resource_desc.temporalLayers = image_handles.size();

        const auto image_handles_ptr = image_handles.data();
        const auto resource_desc_ptr = &resource_desc;

        RpsRenderGraphUpdateInfo render_graph_update_info {};
        render_graph_update_info.gpuCompletedFrameIndex = RPS_GPU_COMPLETED_FRAME_INDEX_NONE;
        render_graph_update_info.frameIndex = _swapchain.current_image_index();
        render_graph_update_info.ppArgResources = &image_handles_ptr;
        render_graph_update_info.ppArgs = reinterpret_cast<const RpsConstant*>(&resource_desc_ptr);
        render_graph_update_info.numArgs = 1;
        if(const auto error = rpsRenderGraphUpdate(_render_graph_handle, &render_graph_update_info); error < 0) {
            return kstd::Error {fmt::format("Unable to update renderer: {}", rpsResultGetName(error))};
        }
        return {};
    }

    auto Renderer::operator=(erebos::render::Renderer&& other) noexcept -> Renderer& {
        _render_graph_handle = other._render_graph_handle;
        _swapchain = std::move(other._swapchain);
        _vulkan_context = other._vulkan_context;
        _vulkan_device = other._vulkan_device;
        _timeline_semaphore = other._timeline_semaphore;
        _rendering_done_semaphore = other._rendering_done_semaphore;
        _image_acquired_semaphore = other._image_acquired_semaphore;
        other._render_graph_handle = nullptr;
        other._vulkan_device = nullptr;
        other._timeline_semaphore = nullptr;
        other._rendering_done_semaphore = nullptr;
        other._image_acquired_semaphore = nullptr;
        return *this;
    }
}// namespace erebos::render