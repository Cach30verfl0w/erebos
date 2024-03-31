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

#pragma once
#include <volk.h>

namespace erebos::render::vulkan {
    class Queue final {
        VkQueue _queue_handle;
        uint32_t _family_index;

    public:
        Queue(VkDevice device, uint32_t queue_family_index, uint32_t queue_index) noexcept
            : _queue_handle()
            , _family_index(queue_family_index) {
            ::vkGetDeviceQueue(device, queue_family_index, queue_index, &_queue_handle);
        }
        ~Queue() noexcept = default;
        EREBOS_DEFAULT_MOVE_COPY(Queue);

        [[nodiscard]] inline auto get_family_index() const noexcept -> uint32_t {
            return _family_index;
        }

        [[nodiscard]] inline auto operator*() const noexcept -> VkQueue {
            return _queue_handle;
        }
    };
}// namespace erebos::render::vulkan