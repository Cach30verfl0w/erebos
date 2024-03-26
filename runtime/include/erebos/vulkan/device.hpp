//  Copyright 2024 Cach30verfl0w
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

/**
 * @author Cedric Hammes
 * @since  14/03/2024
 */

#pragma once
#include "erebos/utils.hpp"
#include "erebos/vulkan/context.hpp"
#include <kstd/defaults.hpp>
#include <kstd/option.hpp>
#include <kstd/result.hpp>
#include <tuple>
#include <mimalloc.h>
#include <rps/rps.h>
#include <rps/runtime/vk/rps_vk_runtime.h>
#include <vk_mem_alloc.h>
#include <volk.h>

namespace erebos::vulkan {
    class Device final {
        VkPhysicalDevice _phy_device;
        VkDevice _device;
        std::array<std::pair<uint32_t, VkQueue>, 3> _queues;
        RpsDevice _runtime_device;
        VmaAllocator _allocator;

    public:
        /**
         * This constructor creates a virtual device handle by the specified physical device handle. After that, the
         * Vulkan memory allocator gets initialized.
         *
         * @param physical_device_handle The handle to the physical device
         * @author                       Cedric Hammes
         * @since                        14/03/2024
         */
        Device(const VulkanContext& context, VkPhysicalDevice physical_device_handle);
        Device(Device&& other) noexcept;

        /**
         * This destructor destroys the device, when the device was not destroyed
         * already.
         *
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        ~Device() noexcept;
        KSTD_NO_COPY(Device, Device);

        [[nodiscard]] inline auto get_queue_family_indices() const noexcept -> std::array<uint32_t, 3> {
            return {_queues[0].first, _queues[1].first, _queues[2].first};
        }

        [[nodiscard]] inline auto get_direct_queue() const noexcept -> VkQueue {
            return _queues[0].second;
        }

        [[nodiscard]] inline auto get_compute_queue() const noexcept -> VkQueue {
            return _queues[1].second;
        }

        [[nodiscard]] inline auto get_transfer_queue() const noexcept -> VkQueue {
            return _queues[2].second;
        }

        /**
         * This function returns the handle to the memory allocator used by this
         * device.
         *
         * @return The memory allocator
         * @author Cedric hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto get_allocator() const noexcept -> VmaAllocator {
            return _allocator;
        }

        /**
         * This function returns the handle to the physical device that is representing this
         * device.
         *
         * @return The physical device handle
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto get_physical_device() const noexcept -> VkPhysicalDevice {
            return _phy_device;
        }

        [[nodiscard]] inline auto get_rps_device() const noexcept -> RpsDevice {
            return _runtime_device;
        }

        /**
         * This function returns a raw handle to the vulkan virtual
         * device.
         *
         * @return The raw device handle
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto operator*() const noexcept -> VkDevice {
            return _device;
        }

        auto operator=(Device&& other) noexcept -> Device&;
    };

    /**
     * This function enumerates all devices and returns that device with the biggest local
     * heap.
     *
     * @param context The context of the Vulkan application
     * @return        The found device or an error
     * @author        Cedric Hammes
     * @since         14/03/2024
     */
    [[nodiscard]] auto find_device(const VulkanContext& context) noexcept -> kstd::Result<Device>;
}// namespace erebos::vulkan