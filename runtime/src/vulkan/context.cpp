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

#include "erebos/vulkan/context.hpp"

namespace erebos::vulkan {
    /**
     * This constructor creates a Vulkan instance for this application. If this engine is built in debug mode, this
     * constructor also enables the Vulkan validation layer and the debug utils extension.
     *
     * @param window Reference to the SQL window
     * @author       Cedric Hammes
     * @since        14/03/2024
     */
    VulkanContext::VulkanContext(const Window& window)
        : _instance {}
        , _surface {}
        , _window {&window} {
        const std::vector<const char*> enabled_layers = {
#ifdef BUILD_DEBUG
            "VK_LAYER_KHRONOS_validation"
#endif
        };

        using namespace std::string_literals;
        if(const auto err = volkInitialize(); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to initialize volk: {}", vk_strerror(err))};
        }

        uint32_t version {};
        if(const auto err = vkEnumerateInstanceVersion(&version); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to get instance version: {}", vk_strerror(err))};
        }
        SPDLOG_INFO("Detected Vulkan API Version {}.{}.{}",
                    VK_API_VERSION_MAJOR(version),
                    VK_API_VERSION_MINOR(version),
                    VK_API_VERSION_PATCH(version));

        // Get SDL window vulkan extensions
        uint32_t window_ext_count = 0;
        if(!SDL_Vulkan_GetInstanceExtensions(*window, &window_ext_count, nullptr)) {
            throw std::runtime_error {"Unable to create vulkan context: Unable to get instance extension count"s};
        }

        auto extensions = std::vector<const char*> {window_ext_count};
        if(!SDL_Vulkan_GetInstanceExtensions(*window, &window_ext_count, extensions.data())) {
            throw std::runtime_error {"Unable to create vulkan context: Unable to get instance extension names"s};
        }
        extensions.push_back("VK_KHR_get_surface_capabilities2");

        // Create application information and instance itself
        VkApplicationInfo application_info {};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pEngineName = "Aetherium Engine";
        application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.apiVersion = version;

        VkInstanceCreateInfo instance_create_info {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        instance_create_info.enabledExtensionCount = extensions.size();
        instance_create_info.ppEnabledExtensionNames = extensions.data();
        instance_create_info.enabledLayerCount = enabled_layers.size();
        instance_create_info.ppEnabledLayerNames = enabled_layers.data();
        SPDLOG_INFO("Create Vulkan context instance with {} extension(s) and {} layer(s)", extensions.size(), enabled_layers.size());
        if(const auto err = ::vkCreateInstance(&instance_create_info, nullptr, &_instance); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create Vulkan instance: {}", vk_strerror(err))};
        }
        ::volkLoadInstance(_instance);

        // Create surface
        if(!::SDL_Vulkan_CreateSurface(*window, _instance, &_surface)) {
            throw std::runtime_error {fmt::format("Unable to create vulkan surface: {}", SDL_GetError())};
        }
    }

    VulkanContext::VulkanContext(VulkanContext&& other) noexcept
        : _instance {other._instance}
        , _surface {other._surface}
        , _window {other._window} {
        other._instance = nullptr;
        other._surface = nullptr;
        other._window = nullptr;
    }

    VulkanContext::~VulkanContext() {
        if(_surface != nullptr) {
            ::vkDestroySurfaceKHR(_instance, _surface, nullptr);
            _surface = nullptr;
        }

        if(_instance != nullptr) {
            ::vkDestroyInstance(_instance, nullptr);
            _instance = nullptr;
        }
    }

    auto VulkanContext::operator=(VulkanContext&& other) noexcept -> VulkanContext& {
        _instance = other._instance;
        _surface = other._surface;
        _window = other._window;
        other._instance = nullptr;
        other._surface = nullptr;
        other._window = nullptr;
        return *this;
    }
}// namespace erebos::vulkan