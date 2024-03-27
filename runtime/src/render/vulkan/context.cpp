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

#include "erebos/render/vulkan/context.hpp"

namespace erebos::render::vulkan {
    namespace {
        auto VKAPI_PTR debug_messenger_callback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
                                                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                [[maybe_unused]] void* user_data) -> VkBool32 {
            SPDLOG_ERROR("Vulkan -> {}", callback_data->pMessage);
            return VK_TRUE;
        }
    }// namespace

    VulkanContext::VulkanContext(const erebos::Window& window)
        : _window(&window)
        , _instance_handle()
        , _surface_handle()
        , _debug_messenger(nullptr) {
        const std::vector<const char*> layers = {
#ifdef BUILD_DEBUG
            "VK_LAYER_KHRONOS_validation"
#endif
        };

        if(const auto error = volkInitialize(); error != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to initialize Volk: {}", vk_strerror(error))};
        }

        // Acquire API version of Vulkan
        if(const auto error = vkEnumerateInstanceVersion(&_api_version); error != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to acquire Vulkan API version: {}", vk_strerror(error))};
        }
        SPDLOG_INFO("Detected Vulkan API Version {}.{}.{}",
                    VK_API_VERSION_MAJOR(_api_version),
                    VK_API_VERSION_MINOR(_api_version),
                    VK_API_VERSION_PATCH(_api_version));

        // Get Vulkan extensions
        using namespace std::string_literals;
        uint32_t window_extension_count = 0;
        if(!SDL_Vulkan_GetInstanceExtensions(*window, &window_extension_count, nullptr)) {
            throw std::runtime_error {"Unable to create vulkan context: Unable to get instance extension count"s};
        }

        auto extensions = std::vector<const char*>(window_extension_count);
        if(!SDL_Vulkan_GetInstanceExtensions(*window, &window_extension_count, extensions.data())) {
            throw std::runtime_error {"Unable to create vulkan context: Unable to get instance extension names"s};
        }

        extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
#ifdef BUILD_DEBUG
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        // Create application info and instance
        VkApplicationInfo application_info {};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pEngineName = "Erebos Engine";
        application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.apiVersion = _api_version;

        VkInstanceCreateInfo instance_create_info {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        instance_create_info.enabledExtensionCount = extensions.size();
        instance_create_info.ppEnabledExtensionNames = extensions.data();
        instance_create_info.enabledLayerCount = layers.size();
        instance_create_info.ppEnabledLayerNames = layers.data();
        if(const auto error = ::vkCreateInstance(&instance_create_info, nullptr, &_instance_handle); error != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create Vulkan instance: {}", vk_strerror(error))};
        }
        SPDLOG_INFO("Successfully created instance for Vulkan Context (Extensions = {}, Layers = {})", extensions.size(), layers.size());
        ::volkLoadInstance(_instance_handle);

#ifdef BUILD_DEBUG
        // Create debug utils messenger if debug build
        VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info {};
        debug_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_messenger_create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_messenger_create_info.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debug_messenger_create_info.pfnUserCallback = debug_messenger_callback;
        if(const auto err = ::vkCreateDebugUtilsMessengerEXT(_instance_handle, &debug_messenger_create_info, nullptr, &_debug_messenger);
           err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to initialize debug messenger: {}", vk_strerror(err))};
        }
#endif

        // Create surface
        if(!::SDL_Vulkan_CreateSurface(*window, _instance_handle, &_surface_handle)) {
            throw std::runtime_error {fmt::format("Unable to create vulkan surface: {}", SDL_GetError())};
        }
    }

    VulkanContext::VulkanContext(VulkanContext&& other) noexcept
        : _window(other._window)
        , _api_version(other._api_version)
        , _instance_handle(other._instance_handle)
        , _debug_messenger(other._debug_messenger)
        , _surface_handle(other._surface_handle) {
        other._instance_handle = nullptr;
        other._debug_messenger = nullptr;
        other._surface_handle = nullptr;
    }

    VulkanContext::~VulkanContext() noexcept {
#ifdef BUILD_DEBUG
        if(_debug_messenger != nullptr) {
            ::vkDestroyDebugUtilsMessengerEXT(_instance_handle, _debug_messenger, nullptr);
            _debug_messenger = nullptr;
        }
#endif

        if(_surface_handle != nullptr) {
            ::vkDestroySurfaceKHR(_instance_handle, _surface_handle, nullptr);
            _surface_handle = nullptr;
        }

        if(_instance_handle != nullptr) {
            ::vkDestroyInstance(_instance_handle, nullptr);
            _instance_handle = nullptr;
        }
    }

    auto VulkanContext::operator=(VulkanContext&& other) noexcept -> VulkanContext& {
        _window = other._window;
        _api_version = other._api_version;
        _instance_handle = other._instance_handle;
        _debug_messenger = other._debug_messenger;
        _surface_handle = other._surface_handle;
        other._instance_handle = nullptr;
        other._debug_messenger = nullptr;
        other._surface_handle = nullptr;
        return *this;
    }
}// namespace erebos::render::vulkan