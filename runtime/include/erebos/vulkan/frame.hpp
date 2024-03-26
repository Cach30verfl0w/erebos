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

#include "erebos/vulkan/command.hpp"
#include "erebos/vulkan/sync/semaphore.hpp"

namespace erebos::vulkan {
    class QueueFrame final {
        Device* _device;
        sync::Semaphore _timeline_semaphore;
        std::vector<CommandBuffer> _recording_command_buffers;
        CommandPool _command_pool;
        VkQueue _queue;

    public:
        QueueFrame(Device& device, [[maybe_unused]] uint32_t queue_family_index);
        ~QueueFrame() noexcept = default;
        KSTD_DEFAULT_MOVE(QueueFrame, QueueFrame);
        KSTD_NO_COPY(QueueFrame, QueueFrame);

        [[nodiscard]] auto begin() const noexcept -> kstd::Result<void>;
        auto end() noexcept -> void;

        [[nodiscard]] inline auto acquire_command_buffer() noexcept -> CommandBuffer& {
            // I know, it's not good but I'm lazy
            _recording_command_buffers.push_back(std::move(_command_pool.allocate(1)->at(0)));
            return _recording_command_buffers.at(_recording_command_buffers.size() - 1);
        }

        [[nodiscard]] inline auto get_timeline_semaphore() const noexcept -> const sync::Semaphore& {
            return _timeline_semaphore;
        }

        [[nodiscard]] inline auto get_queue() const noexcept -> VkQueue {
            return _queue;
        }
    };

    class RenderFrame {
        sync::Semaphore _rendering_done_semaphore;
        sync::Fence _fence;
        std::vector<QueueFrame> _queue_frames;

    public:
        explicit RenderFrame(Device& device);
        ~RenderFrame() noexcept = default;
        KSTD_DEFAULT_MOVE(RenderFrame, RenderFrame);
        KSTD_NO_COPY(RenderFrame, RenderFrame);

        [[nodiscard]] auto begin() const noexcept -> kstd::Result<void>;
        auto end() noexcept -> void;

        [[nodiscard]] inline auto get_rendering_done_semaphore() const noexcept -> const sync::Semaphore& {
            return _rendering_done_semaphore;
        }

        [[nodiscard]] inline auto queue_frame_at(uint32_t queue_frame_index) const noexcept -> const QueueFrame& {
            return _queue_frames.at(queue_frame_index);
        }

        [[nodiscard]] inline auto queue_frame_at(uint32_t queue_frame_index) noexcept -> QueueFrame& {
            return _queue_frames.at(queue_frame_index);
        }

        [[nodiscard]] inline auto get_fence() const noexcept -> const sync::Fence& {
            return _fence;
        }
    };
}