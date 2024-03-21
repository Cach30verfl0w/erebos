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
 * @since  14/03/2024
 */

#pragma once
#include "erebos/vulkan/device.hpp"
#include <chrono>
#include <volk.h>

namespace erebos::vulkan {
    /**
     * This is a wrapper around the fence, provided by Vulkan. Fences are allowing the developer to wait on the CPU-side
     * for the end of GPU-side operations like queue submits etc.
     *
     * @author Cedric Hammes
     * @since  14/03/2024
     */
    class Fence final {
        const Device* _device;
        VkFence _fence;

    public:
        /**
         * This constructor creates a new fence on the specified device. This function throws an exception when the
         * creation doesn't works.
         *
         * @param device Reference to device
         * @author       Cedric Hammes
         * @since        24/03/2024
         */
        explicit Fence(const Device& device);
        Fence(Fence&& other) noexcept;
        ~Fence() noexcept;
        KSTD_NO_COPY(Fence, Fence);

        /**
         * This function waits for a signal by the fence that indicates that the task has ended. This function waits
         * until a signal has been sent or the timeout was exceeded.
         *
         * @tparam Rep
         * @tparam Period
         * @param timeout The maximal timeout of this wait
         * @return        Void or an error
         * @author        Cedric Hammes
         * @since         24/03/2024
         */
        template<typename Rep = int, typename Period = std::ratio<std::numeric_limits<Rep>::max()>>
        [[nodiscard]] auto wait_for(const std::chrono::duration<Rep, Period>& timeout =
                                            std::chrono::duration<Rep, Period>::max()) const noexcept -> VoidResult {
            const auto timeout_millis = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
            if(const auto err = vkWaitForFences(**_device, 1, &_fence, true, timeout_millis); err != VK_SUCCESS) {
                return kstd::Error {fmt::format("Unable to wait for fence: {}", vk_strerror(err))};
            }
            return {};
        }

        /**
         * This function returns a reference to the original vulkan fence
         * handle.
         *
         * @return The raw fence
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto operator*() const noexcept -> VkFence {
            return _fence;
        }

        auto operator=(Fence&& other) noexcept -> Fence&;
    };
}// namespace erebos::vulkan