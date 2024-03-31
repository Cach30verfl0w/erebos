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
    class Semaphore {
        const Device* _device;
        VkSemaphore _handle;

    public:
        /**
         * Initialize the semaphore (as timeline semaphore if is_timeline is set to true) and set the device pointer
         * internally.
         *
         * @param device      Reference to the device
         * @param is_timeline Whether this semaphore is a timeline semaphore
         * @author            Cedric Hammes
         * @since             28/03/2024
         */
        Semaphore(const Device& device, const bool is_timeline = false)
            : _device(&device)
            , _handle() {
            VkSemaphoreCreateInfo semaphore_create_info {};
            if(is_timeline) {
                VkSemaphoreTypeCreateInfo semaphore_type_create_info {};
                semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
                semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
                semaphore_type_create_info.initialValue = 0;
                semaphore_create_info.pNext = &semaphore_type_create_info;
            }
            semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            if (const auto error = ::vkCreateSemaphore(**_device, &semaphore_create_info, nullptr, &_handle); error != VK_SUCCESS) {
                throw std::runtime_error(fmt::format("Unable to create semaphore: {}", vk_strerror(error)));
            }
        }

        Semaphore(Semaphore&& other) noexcept
            : _device(other._device)
            , _handle(other._handle) {
            other._device = nullptr;
            other._handle = nullptr;
        }

        EREBOS_DELETE_COPY(Semaphore);

        auto operator=(Semaphore&& other) noexcept -> Semaphore& {
            _device = other._device;
            _handle = other._handle;
            other._device = nullptr;
            other._handle = nullptr;
            return *this;
        }

        /**
         * This operator overload function returns the handle to the vulkan
         * semaphore.
         *
         * @return The semaphore handle
         * @author Cedric Hammes
         * @since  28/03/2024
         */
        [[nodiscard]] inline auto operator*() const noexcept -> VkSemaphore {
            return _handle;
        }
    };
}// namespace erebos::render::vulkan::sync