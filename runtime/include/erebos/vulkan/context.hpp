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
#include "erebos/utils.hpp"
#include "erebos/window.hpp"
#include <SDL2/SDL_vulkan.h>
#include <volk.h>

namespace erebos::vulkan {
    /**
     * This class holds the context information for the Vulkan API. This class mainly holds the application instance of
     * Vulkan and the vulkan surface, created by the window.
     *
     * @author Cedric Hammes
     * @since  14/03/2024
     */
    class VulkanContext final {
        VkInstance _instance;
        VkSurfaceKHR _surface;
        const Window* _window;

        public:
        /**
         * This constructor creates a Vulkan instance for this application. If this engine is built in debug mode, this
         * constructor also enables the Vulkan validation layer and the debug utils extension.
         *
         * TODO: Add debug utils messenger
         *
         * @param window Reference to the SQL window
         * @author       Cedric Hammes
         * @since        14/03/2024
         */
        explicit VulkanContext(const Window& window);
        VulkanContext(VulkanContext&& other) noexcept;
        ~VulkanContext();
        KSTD_NO_COPY(VulkanContext, VulkanContext);

        /**
         * This function returns the handle to the vulkan surface, created with the
         * window.
         *
         * @return The surface handle
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto get_surface() const noexcept -> VkSurfaceKHR {
            return _surface;
        }

        /**
         * This function returns a pointer to the window. This window was used to initialize this Vulkan
         * context.
         *
         * @return Pointer to window
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto get_window() const noexcept -> const Window* {
            return _window;
        }

        /**
         * This function returns the raw handle to the Vulkan API
         * instance.
         *
         * @return The raw instance handle
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto operator*() const noexcept -> VkInstance {
            return _instance;
        }

        auto operator=(VulkanContext&& other) noexcept -> VulkanContext&;
    };
}// namespace erebos::vulkan