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
#include "erebos/render/vulkan/context.hpp"
#include "erebos/render/vulkan/queue.hpp"
#include "erebos/utils.hpp"
#include <mimalloc.h>
#include <optional>
#include <vk_mem_alloc.h>

namespace erebos::render::vulkan {
    class Device final {
        const VulkanContext* _context;
        VkPhysicalDevice _physical_device;
        VkDevice _device_handle;
        RpsDevice _rps_device;
        VmaAllocator _allocator;
        std::vector<Queue> _queues;

    public:
        /**
         * This constructor creates a virtual device handle by the specified physical device handle. It also creates the Vulkan memory
         * allocator and the RPS (RenderPipelineShaders) device gets initialized.
         *
         * @param vulkan_context  The context of the vulkan API
         * @param physical_device The handle of the physical device
         * @author                Cedric Hammes
         * @since                 26/03/2024
         */
        Device(const VulkanContext& vulkan_context, VkPhysicalDevice physical_device);
        Device(Device&& other) noexcept;
        ~Device() noexcept;
        EREBOS_DELETE_COPY(Device);
        auto operator=(Device&& other) noexcept -> Device&;

        /**
         * This function returns the list of the allocated queues
         *
         * @return The queue list
         * @author Cedric Hammes
         * @since  27/03/2024
         */
        [[nodiscard]] inline auto get_queues() const noexcept -> const std::vector<Queue>& {
            return _queues;
        }

        /**
         * This function returns the handle of the device's physical device
         *
         * @return The physical device handle
         * @author Cedric Hammes
         * @since  27/03/2024
         */
        [[nodiscard]] inline auto get_physical_device() const noexcept -> VkPhysicalDevice {
            return _physical_device;
        }

        /**
         * This function returns the handle of the RPS (RenderPipelineShaders) runtime device
         *
         * @return The handle of the RPS runtime device
         * @author Cedric Hammes
         * @since  27/03/2024
         */
        [[nodiscard]] inline auto get_rps_device() const noexcept -> RpsDevice {
            return _rps_device;
        }

        /**
         * This function returns the handle of the device's VMA allocator
         *
         * @return The device's allocator handle
         * @author Cedric Hammes
         * @since  27/03/2024
         */
        [[nodiscard]] inline auto get_allocator() const noexcept -> VmaAllocator {
            return _allocator;
        }

        /**
         * This operator function returns the handle of the virtual device.
         *
         * @return The virtual device handle
         * @author Cedric Hammes
         * @since  27/03/2024
         */
        [[nodiscard]] inline auto operator*() const noexcept -> VkDevice {
            return _device_handle;
        }
    };

    [[nodiscard]] auto find_preferred_device(const VulkanContext& context) noexcept -> std::optional<Device>;
}// namespace erebos::render::vulkan