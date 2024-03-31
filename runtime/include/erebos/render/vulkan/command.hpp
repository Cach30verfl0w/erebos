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
 * @since  31/03/2024
 */

#pragma once
#include "erebos/platform/platform.hpp"
#include "erebos/render/vulkan/command.hpp"
#include "erebos/render/vulkan/sync/fence.hpp"

namespace erebos::render::vulkan {
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
        EREBOS_DELETE_COPY(CommandBuffer);

        [[nodiscard]] auto begin(VkCommandBufferUsageFlags usage = 0) const noexcept -> Result<void>;
        [[nodiscard]] auto end() const noexcept -> Result<void>;

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
        CommandPool(const Device& device, uint32_t queue_family_index);
        CommandPool(CommandPool&& other) noexcept;

        /**
         * This destructor destroys the command pool if the handle is valid
         *
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        ~CommandPool() noexcept;
        EREBOS_DELETE_COPY(CommandPool);

        /**
         * This function allocates the specified count of wrapped command buffers
         *
         * @param count The count of newly allocated command buffers
         * @return      The command buffers or an error
         * @author      Cedric Hammes
         * @since       14/03/2024
         */
        [[nodiscard]] auto allocate(uint32_t count) const noexcept -> Result<std::vector<CommandBuffer>>;

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
        auto emit_command_buffer(F&& function) const noexcept -> Result<void> {
            static_assert(std::is_convertible_v<F, std::function<void(CommandBuffer&)>>, "Invalid command buffer consumer");

            // Create command buffer and submit fence
            const auto command_buffers = allocate(1);
            if(command_buffers.is_error()) {
                return Error(command_buffers.get_error());
            }
            const auto submit_fence = sync::Fence(*_device);
            const auto command_buffer = &command_buffers.get()[0];
            const auto raw_command_buffer = **command_buffer;

            // Perform operation
            if(auto begin_result = command_buffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT); begin_result.is_error()) {
                return begin_result;
            }
            function(&command_buffer);
            if(auto end_result = command_buffer->end(); end_result.is_error()) {
                return end_result;
            }


            // Submit
            VkSubmitInfo submit_info {};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pCommandBuffers = &raw_command_buffer;
            submit_info.commandBufferCount = 1;
            if(auto error = vkQueueSubmit(*_device->get_queues()[0], 1, &submit_info, *submit_fence); error != VK_SUCCESS) {
                return Error(fmt::format("Unable to emit one-time command buffer: {}", platform::get_last_error()));
            }

            if(auto wait_result = submit_fence.wait(); wait_result.is_error()) {
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
}// namespace erebos::render::vulkan