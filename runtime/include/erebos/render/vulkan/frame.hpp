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
 * @since  31/03/2024
 */

#pragma once
#include "erebos/render/vulkan/command.hpp"
#include "erebos/render/vulkan/device.hpp"
#include "erebos/render/vulkan/queue.hpp"
#include "erebos/render/vulkan/sync/fence.hpp"
#include "erebos/render/vulkan/sync/semaphore.hpp"

namespace erebos::render::vulkan {
    class QueueFrame final {
        Device const* _device;
        sync::Semaphore _timeline_semaphore;
        CommandPool _command_pool;
        std::vector<CommandBuffer> _recording_command_buffers;
        std::vector<CommandBuffer> _cached_command_buffers;
        Queue const* _queue;

    public:
        QueueFrame(Device const& device, Queue const& queue)
            : _device(&device)
            , _timeline_semaphore(device)
            , _command_pool(device, queue.get_family_index())
            , _queue(&queue)
            , _recording_command_buffers()
            , _cached_command_buffers() {
        }
        EREBOS_DEFAULT_MOVE(QueueFrame);
        EREBOS_DELETE_COPY(QueueFrame);

        [[nodiscard]] auto acquire_command_buffer() noexcept -> Result<CommandBuffer*>;

        [[nodiscard]] inline auto get_recording_command_buffers() noexcept -> std::vector<CommandBuffer>& {
            return _recording_command_buffers;
        }

        [[nodiscard]] inline auto get_cached_command_buffers() noexcept -> std::vector<CommandBuffer>& {
            return _cached_command_buffers;
        }

        [[nodiscard]] inline auto get_command_pool() const noexcept -> const CommandPool& {
            return _command_pool;
        }
    };

    class Frame final {
        Device const* _device;
        sync::Semaphore _image_acquired_semaphore;
        sync::Semaphore _rendering_done_semaphore;
        sync::Fence _queue_submit_fence;
        std::vector<QueueFrame> _queue_frames;

    public:
        explicit Frame(Device const& device)
            : _device(&device)
            , _image_acquired_semaphore(device)
            , _rendering_done_semaphore(device)
            , _queue_submit_fence(device, true)
            , _queue_frames() {
            _queue_frames.reserve(device.get_queues().size());
            for(const auto& queue : device.get_queues()) {
                _queue_frames.emplace_back(device, queue);
            }
        }
        ~Frame() noexcept = default;
        EREBOS_DEFAULT_MOVE(Frame);
        EREBOS_DELETE_COPY(Frame);

        [[nodiscard]] auto begin_frame() noexcept -> Result<void>;

        [[nodiscard]] inline auto get_image_acquired_semaphore() const noexcept -> const sync::Semaphore& {
            return _image_acquired_semaphore;
        }

        [[nodiscard]] inline auto get_rendering_done_semaphore() const noexcept -> const sync::Semaphore& {
            return _rendering_done_semaphore;
        }

        [[nodiscard]] inline auto get_queue_submit_fence() const noexcept -> const sync::Fence& {
            return _queue_submit_fence;
        }
    };
}// namespace erebos::render::vulkan