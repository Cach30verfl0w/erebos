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
 * @since  28/03/2024
 */

#pragma once
#include "erebos/render/vulkan/device.hpp"

namespace erebos::render::vulkan::sync {
    /**
     * This class is a safe-wrapper around the Vulkan fence. This class automatically frees the resource and provides
     * simple functions for waiting and reset the fence itself.
     *
     * @author Cedric Hammes
     * @since  28/03/2024
     */
    class Fence {
        const Device* _device;
        VkFence _handle;

    public:
        /**
         * Initialize the fence (and set it signaled if flag was set) and stores a pointer of the device in the fence
         * itself.
         *
         * @param device      Reference to the device
         * @param is_signaled Whether set the fence already signaled or not
         * @author            Cedric Hammes
         * @since             28/03/2024
         */
        Fence(const Device& device, const bool is_signaled = false)
            : _device(&device)
            , _handle() {
            VkFenceCreateInfo fence_create_info {};
            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create_info.flags = is_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0x0;
            if(const auto error = ::vkCreateFence(**_device, &fence_create_info, nullptr, &_handle); error != VK_SUCCESS) {
                throw std::runtime_error(fmt::format("Unable to create fence: {}", vk_strerror(error)));
            }
        }

        Fence(Fence&& other) noexcept
            : _device(other._device)
            , _handle(other._handle) {
            other._device = nullptr;
            other._handle = nullptr;
        }

        EREBOS_DELETE_COPY(Fence);

        /**
         * This function awaits the semaphore to be signaled as indicator that a task ended etc. If the timeout exceeds
         * this
         * function returns a error.
         *
         * @tparam TRep    The data format to store the timeout
         * @tparam TPeriod The type of period for the timeout
         * @param timeout  The timeout value itself
         * @return         Void or an error
         * @author         Cedric Hammes
         * @since          28/03/2024
         */
        // clang-format off
        template<typename TRep = std::int64_t, typename TPeriod = std::ratio<std::numeric_limits<TRep>::max()>>
        [[nodiscard]] auto wait(const std::chrono::duration<TRep, TPeriod> timeout = std::chrono::duration<TRep, TPeriod>::max())
            const noexcept -> Result<void> {
            constexpr auto timeout_millis = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
            if(const auto error = ::vkWaitForFences(**_device, 1, &_handle, true, timeout_millis)) {
                return Error(fmt::format("Unable to wait for fence to be signaled: {}", vk_strerror(error)));
            }
            return {};
        }
        // clang-format on

        auto operator=(Fence&& other) noexcept -> Fence& {
            _device = other._device;
            _handle = other._handle;
            other._device = nullptr;
            other._handle = nullptr;
            return *this;
        }

        /**
         * This operator overload function returns the handle to the vulkan
         * fence.
         *
         * @return The fence handle
         * @author Cedric Hammes
         * @since  28/03/2024
         */
        [[nodiscard]] inline auto operator*() const noexcept -> VkFence {
            return _handle;
        }
    };
}// namespace erebos::render::vulkan::sync