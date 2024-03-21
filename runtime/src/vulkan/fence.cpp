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

#include "erebos/vulkan/fence.hpp"

namespace erebos::vulkan {
    /**
     * This constructor creates a new fence on the specified device. This function throws an exception when the
     * creation doesn't works.
     *
     * @param device Reference to device
     * @author       Cedric Hammes
     * @since        24/03/2024
     */
    Fence::Fence(const Device& device) ://NOLINT
            _device {&device},
            _fence {} {
        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        if(const auto err = ::vkCreateFence(**_device, &fence_create_info, nullptr, &_fence); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create fence: {}", vk_strerror(err))};
        }
    }

    Fence::Fence(Fence&& other) noexcept :
            _device {other._device},
            _fence {other._fence} {
        other._device = nullptr;
        other._fence = nullptr;
    }

    Fence::~Fence() noexcept {
        if(_device != nullptr && _fence != nullptr) {
            ::vkDestroyFence(**_device, _fence, nullptr);
            _device = nullptr;
            _fence = nullptr;
        }
    }

    auto Fence::operator=(Fence&& other) noexcept -> Fence& {
        _device = other._device;
        _fence = other._fence;
        other._device = nullptr;
        other._fence = nullptr;
        return *this;
    }
}// namespace erebos::vulkan