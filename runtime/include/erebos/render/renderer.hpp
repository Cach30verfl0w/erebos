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
 * @since  15/03/2024
 */

#pragma once
#include "erebos/vulkan/command.hpp"
#include "erebos/vulkan/device.hpp"
#include "erebos/vulkan/swapchain.hpp"
#include "erebos/vulkan/sync/fence.hpp"
#include <rps/core/rps_api.h>

namespace erebos::render {
    class Renderer {
        RpsRenderGraph _render_graph_handle;
        const vulkan::VulkanContext* _vulkan_context;
        const vulkan::Device* _vulkan_device;
        vulkan::Swapchain _swapchain;
        VkSemaphore _timeline_semaphore;
        VkSemaphore _rendering_done_semaphore;
        VkSemaphore _image_acquired_semaphore;

    public:
        Renderer(const vulkan::VulkanContext& context, const vulkan::Device& device);
        Renderer(Renderer&& other) noexcept;
        ~Renderer() noexcept;
        KSTD_NO_COPY(Renderer, Renderer);

        [[nodiscard]] auto render() const noexcept -> kstd::Result<void>;
        [[nodiscard]] auto update() const noexcept -> kstd::Result<void>;

        inline auto operator=(Renderer&&) noexcept -> Renderer&;
    };
}// namespace erebos::render