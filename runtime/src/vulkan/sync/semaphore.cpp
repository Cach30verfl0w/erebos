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

#include "erebos/vulkan/sync/semaphore.hpp"

namespace erebos::vulkan::sync {
    Semaphore::Semaphore(const Device& device, bool is_timeline)
        : _device {&device} {
        VkSemaphoreCreateInfo semaphore_create_info {};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if(is_timeline) {
            VkSemaphoreTypeCreateInfo semaphore_type_create_info {};
            semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
            semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            semaphore_type_create_info.initialValue = 0;
            semaphore_create_info.pNext = &semaphore_type_create_info;
        }

        if(const auto error = vkCreateSemaphore(*device, &semaphore_create_info, nullptr, &_handle)) {
            throw std::runtime_error {fmt::format("Unable to create renderer: {}", vk_strerror(error))};
        }
    }

    Semaphore::Semaphore(Semaphore&& other) noexcept
        : _device {other._device}
        , _handle {other._handle} {
        other._device = nullptr;
        other._handle = nullptr;
    }

    Semaphore::~Semaphore() noexcept {
        if(_handle != nullptr) {
            vkDestroySemaphore(**_device, _handle, nullptr);
            _handle = nullptr;
        }
    }

    auto Semaphore::operator=(Semaphore&& other) noexcept -> Semaphore& {
        _device = other._device;
        _handle = other._handle;
        other._device = nullptr;
        other._handle = nullptr;
        return *this;
    }
}// namespace erebos::vulkan::sync