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

#include "erebos/vulkan/swapchain.hpp"

namespace erebos::vulkan {
    Swapchain::Swapchain(const VulkanContext& context, const Device& device) ://NOLINT
            _device {&device},
            _swapchain {},
            _images {},
            _image_views {} {
        int32_t width = 0;
        int32_t height = 1;
        ::SDL_GetWindowSize(**context.get_window(), &width, &height);
        VkExtent2D window_size {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        // Create swapchain
        VkSwapchainCreateInfoKHR create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = context.get_surface();
        create_info.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
        create_info.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        create_info.minImageCount = 2;
        create_info.imageArrayLayers = 1;
        create_info.imageExtent = window_size;
        if(const auto err = ::vkCreateSwapchainKHR(*device, &create_info, nullptr, &_swapchain); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create swapchain: {}", vk_strerror(err))};
        }

        // Get images
        uint32_t image_count = 0;
        if(const auto err = ::vkGetSwapchainImagesKHR(*device, _swapchain, &image_count, nullptr); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to get swapchain image count: {}", vk_strerror(err))};
        }

        _images.resize(image_count);
        _image_views.resize(image_count);
        if(const auto err = ::vkGetSwapchainImagesKHR(*device, _swapchain, &image_count, _images.data());
           err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to get swapchain images: {}", vk_strerror(err))};
        }

        for(auto i = 0; i < image_count; i++) {
            VkImageViewCreateInfo image_view_info {};
            image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_info.image = _images[i];
            image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_info.format = VK_FORMAT_B8G8R8A8_UNORM;
            image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_info.subresourceRange.baseMipLevel = 0;
            image_view_info.subresourceRange.levelCount = 1;
            image_view_info.subresourceRange.baseArrayLayer = 0;
            image_view_info.subresourceRange.layerCount = 1;
            if(const auto err = ::vkCreateImageView(*device, &image_view_info, nullptr, _image_views.data() + i);
               err != VK_SUCCESS) {
                throw std::runtime_error {fmt::format("Unable to create image view from image: {}", vk_strerror(err))};
            }
        }
    }

    Swapchain::Swapchain(Swapchain&& other) noexcept :
            _device {other._device},
            _swapchain {other._swapchain},
            _images {std::move(other._images)},
            _image_views {std::move(other._image_views)} {
        other._device = nullptr;
        other._swapchain = nullptr;
        other._image_views = {};
    }

    Swapchain::~Swapchain() noexcept {
        if(!_image_views.empty()) {
            for(const auto& image_view : _image_views) {
                vkDestroyImageView(**_device, image_view, nullptr);
            }
            _image_views = {};
        }

        if(_swapchain != nullptr) {
            vkDestroySwapchainKHR(**_device, _swapchain, nullptr);
            _swapchain = nullptr;
        }
    }

    auto Swapchain::next_image(VkSemaphore image_available_semaphore) noexcept -> kstd::Result<void> {
        const auto err = vkAcquireNextImageKHR(**_device, _swapchain, std::numeric_limits<uint64_t>::max(),
                                               image_available_semaphore, VK_NULL_HANDLE, &_current_image_index);
        if(err != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to acquire next image: {}", vk_strerror(err))};
        }
        return {};
    }

    auto Swapchain::operator=(Swapchain&& other) noexcept -> Swapchain& {
        _device = other._device;
        _swapchain = other._swapchain;
        _images = std::move(other._images);
        _image_views = std::move(other._image_views);
        other._device = nullptr;
        other._swapchain = nullptr;
        other._image_views = {};
        return *this;
    }
}// namespace erebos::vulkan