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
#include "erebos/utils.hpp"
#include "erebos/window.hpp"
#include <SDL2/SDL_vulkan.h>
#include <volk.h>

namespace erebos::render::vulkan {
    class VulkanContext final {
        const Window* _window;
        VkInstance _instance_handle;
        VkSurfaceKHR _surface_handle;
        uint32_t _api_version;
#ifdef BUILD_DEBUG
        VkDebugUtilsMessengerEXT _debug_messenger;
#endif

    public:
        explicit VulkanContext(const Window& window);
        VulkanContext(VulkanContext&& other) noexcept;
        ~VulkanContext() noexcept;
        EREBOS_DELETE_COPY(VulkanContext);
        auto operator=(VulkanContext&& other) noexcept -> VulkanContext&;

        /**
         * This function returns the currently available Vulkan API version. This is used for the initialization
         * of some components.
         *
         * @return The Vulkan API version
         * @author Cedric Hammes
         * @since  28/03/2024
         */
        [[nodiscard]] inline auto get_api_version() const noexcept -> uint32_t {
            return _api_version;
        }

        /**
         * This function returns a non-mutable pointer to the window, that was used to create this Vulkan API
         * context.
         *
         * @return The window
         * @author Cedric Hammes
         * @since  28/03/2024
         */
        [[nodiscard]] inline auto get_window() const noexcept -> const Window* {
            return _window;
        }

        [[nodiscard]] inline auto get_surface() const noexcept -> VkSurfaceKHR {
            return _surface_handle;
        }

        [[nodiscard]] inline auto operator*() const noexcept -> VkInstance {
            return _instance_handle;
        }
    };
}// namespace erebos::render::vulkan