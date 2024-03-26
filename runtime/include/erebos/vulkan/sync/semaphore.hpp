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
 * @since  25/03/2024
 */
#pragma once
#include "erebos/vulkan/device.hpp"

namespace erebos::vulkan::sync {
    class Semaphore final {
        const Device* _device;
        VkSemaphore _handle;

    public:
        Semaphore(const Device& device, bool is_timeline);
        Semaphore(Semaphore&& other) noexcept;
        ~Semaphore() noexcept;
        KSTD_NO_COPY(Semaphore, Semaphore);

        inline auto operator*() const noexcept -> VkSemaphore {
            return _handle;
        }

        auto operator=(Semaphore&& other) noexcept -> Semaphore&;
    };
}