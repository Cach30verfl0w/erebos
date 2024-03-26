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
 * @author BeastLe9end (https://github.com/ProjectKML/greek/blob/main/crates/greek_render/src/renderer/device_frame.rs), Cedric Hammes
 * @since  25/03/2024
 */

#include "erebos/vulkan/frame.hpp"

namespace erebos::vulkan {
    QueueFrame::QueueFrame(erebos::vulkan::Device& device, uint32_t queue_family_index)
        : _timeline_semaphore {sync::Semaphore(device, true)}
        , _command_pool {CommandPool {device, queue_family_index}}
        , _queue {}
        , _device {&device}
        , _recording_command_buffers {} {
        vkGetDeviceQueue(*device, queue_family_index, 0, &_queue);
    }

    auto QueueFrame::begin() const noexcept -> kstd::Result<void> {
        if(const auto error = vkResetCommandPool(**_device, *_command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
           error != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to begin frame: {}", vk_strerror(error))};
        }
        return {};
    }

    auto QueueFrame::end() noexcept -> void {
        _recording_command_buffers.clear();
    }

    RenderFrame::RenderFrame(Device& device)
        : _rendering_done_semaphore {sync::Semaphore {device, false}}
        , _fence {sync::Fence(device)}
        , _queue_frames {} {
        for(const auto queue_family_index : device.get_queue_family_indices()) {
            _queue_frames.emplace_back(device, queue_family_index);
        }
    }

    auto RenderFrame::begin() const noexcept -> kstd::Result<void> {
        for(const auto& queue_frame : _queue_frames) {
            if(const auto result = queue_frame.begin(); !result) {
                return kstd::Error {result.get_error()};
            }
        }
        return {};
    }

    auto RenderFrame::end() noexcept -> void {
        for(auto& queue_frame : _queue_frames) {
            queue_frame.end();
        }
    }
}// namespace erebos::vulkan