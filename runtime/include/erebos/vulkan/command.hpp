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
#include "erebos/platform/platform.hpp"
#include "erebos/vulkan/device.hpp"
#include "erebos/vulkan/fence.hpp"
#include <kstd/safe_alloc.hpp>

namespace erebos::vulkan {
    class CommandPool;

    class CommandBuffer final {
        const CommandPool* _command_pool;
        VkCommandBuffer _command_buffer;

        public:
        /**
         * This constructor initializes the values of this wrapper with the specified command pool and command
         * buffer.
         *
         * @param command_pool   The creator pool of this command buffer
         * @param command_buffer The created command buffer
         * @author               Cedric Hammes
         * @since                14/03/2024
         */
        CommandBuffer(const CommandPool* command_pool, VkCommandBuffer command_buffer) noexcept;
        CommandBuffer(CommandBuffer&& other) noexcept;

        /**
         * This destructor destroys the command buffer if the handle is valid
         *
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        ~CommandBuffer() noexcept;
        KSTD_NO_COPY(CommandBuffer, CommandBuffer);

        [[nodiscard]] auto begin(VkCommandBufferUsageFlags usage = 0) const noexcept -> kstd::Result<void>;
        [[nodiscard]] auto end() const noexcept -> kstd::Result<void>;

        /**
         * This function returns the raw handle to the command buffer
         *
         * @return The raw command buffer handle
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        inline auto operator*() const noexcept -> VkCommandBuffer {
            return _command_buffer;
        }

        auto operator=(CommandBuffer&& other) noexcept -> CommandBuffer&;
    };

    class CommandPool final {
        const Device* _device;
        VkCommandPool _command_pool;

        friend class CommandBuffer;

        public:
        /**
         * This constructor creates a command pool on the specified device
         *
         * @param device The device for create the pool
         * @author       Cedric Hammes
         * @since        14/03/2024
         */
        explicit CommandPool(const Device& device);
        CommandPool(CommandPool&& other) noexcept;

        /**
         * This destructor destroys the command pool if the handle is valid
         *
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        ~CommandPool() noexcept;
        KSTD_NO_COPY(CommandPool, CommandPool);

        /**
         * This function allocates the specified count of wrapped command buffers
         *
         * @param count The count of newly allocated command buffers
         * @return      The command buffers or an error
         * @author      Cedric Hammes
         * @since       14/03/2024
         */
        [[nodiscard]] auto allocate(uint32_t count) const noexcept -> kstd::Result<std::vector<CommandBuffer>>;

        /**
         * This function creates a one-time command buffer and executes the specified function. After the run, the
         * command buffer get submitted into the queue and the program waits for the execution.
         *
         * @tparam F       The function type
         * @param function The function itself
         * @return         Void or an error
         * @author         Cedric Hammes
         * @since          06/02/2024
         */
        template<typename F>
        auto emit_command_buffer(F&& function) const noexcept -> kstd::Result<void> {
            static_assert(std::is_convertible_v<F, std::function<void(CommandBuffer&)>>, "Invalid command buffer consumer");

            // Create command buffer and submit fence
            const auto command_buffer = std::move(allocate(1).get_or_throw()[0]);
            const auto submit_fence = Fence {*_device};
            const auto raw_command_buffer = *command_buffer;

            // Perform operation
            if(const auto begin_result = command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT); begin_result.is_error()) {
                return begin_result;
            }
            function(&command_buffer);
            if(const auto end_result = command_buffer.end(); end_result.is_error()) {
                return end_result;
            }

            // Submit
            VkSubmitInfo submit_info {};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pCommandBuffers = &raw_command_buffer;
            submit_info.commandBufferCount = 1;
            if(const auto error = vkQueueSubmit(_device->get_graphics_queue(), 1, &submit_info, *submit_fence); error != VK_SUCCESS) {
                return kstd::Error {fmt::format("Unable to emit one-time command buffer: {}", platform::get_last_error())};
            }

            if(const auto wait_result = submit_fence.wait_for(); wait_result.is_error()) {
                return wait_result;
            }
            return {};
        }

        /**
         * This function returns the raw handle to the command pool
         *
         * @return The raw command pool handle
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        auto operator*() const noexcept -> VkCommandPool {
            return _command_pool;
        }

        auto operator=(CommandPool&& other) noexcept -> CommandPool&;
    };
}// namespace erebos::vulkan