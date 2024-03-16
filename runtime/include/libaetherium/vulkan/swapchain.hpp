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
#include "libaetherium/vulkan/context.hpp"
#include "libaetherium/vulkan/device.hpp"

namespace libaetherium::vulkan {
    class Swapchain final {
        const Device* _device;
        VkSwapchainKHR _swapchain;
        std::vector<VkImage> _images;
        std::vector<VkImageView> _image_views;
        uint32_t _current_image_index = 0;

    public:
        /**
         * This method initializes the swapchain, acquires the images and creates image views by these acquired
         * images.
         *
         * @param context The vulkan context
         * @param device  The target vulkan device
         * @author        Cedric Hammes
         * @since         14/03/2024
         */
        Swapchain(const VulkanContext& context, const Device& device);
        Swapchain(Swapchain&& other) noexcept;
        ~Swapchain() noexcept;
        KSTD_NO_COPY(Swapchain, Swapchain);

        [[nodiscard]] auto next_image(VkSemaphore image_available_semaphore) noexcept -> kstd::Result<void>;

        [[nodiscard]] inline auto current_image_index() const noexcept -> uint32_t {
            return _current_image_index;
        }

        [[nodiscard]] inline auto current_image() const noexcept -> VkImage {
            return _images.at(_current_image_index);
        }

        [[nodiscard]] inline auto current_image_view() const noexcept -> VkImageView {
            return _image_views.at(_current_image_index);
        }

        [[nodiscard]] inline auto operator*() const noexcept -> VkSwapchainKHR {
            return _swapchain;
        }

        auto operator=(Swapchain&& other) noexcept -> Swapchain&;
    };
}// namespace libaetherium::vulkan