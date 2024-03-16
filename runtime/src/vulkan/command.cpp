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

#include "libaetherium/vulkan/command.hpp"

namespace libaetherium::vulkan {
    /**
     * This constructor initializes the values of this wrapper with the specified command pool and command
     * buffer.
     *
     * @param command_pool   The creator pool of this command buffer
     * @param command_buffer The created command buffer
     * @author               Cedric Hammes
     * @since                14/03/2024
     */
    CommandBuffer::CommandBuffer(const libaetherium::vulkan::CommandPool* command_pool,
                                 VkCommandBuffer command_buffer) noexcept ://NOLINT
            _command_pool {command_pool},
            _command_buffer {command_buffer} {
    }

    CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept :
            _command_pool {other._command_pool},
            _command_buffer {other._command_buffer} {
        other._command_pool = nullptr;
        other._command_buffer = nullptr;
    }

    /**
     * This destructor destroys the command buffer if the handle is valid
     *
     * @author Cedric Hammes
     * @since  14/03/2024
     */
    CommandBuffer::~CommandBuffer() noexcept {
        if(_command_buffer != nullptr) {
            ::vkFreeCommandBuffers(**_command_pool->_device, **_command_pool, 1, &_command_buffer);
            _command_pool = nullptr;
        }
    }

    auto CommandBuffer::begin(VkCommandBufferUsageFlags usage) const noexcept -> kstd::Result<void> {
        VkCommandBufferBeginInfo command_buffer_begin_info {};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = usage;
        if (const auto err = ::vkBeginCommandBuffer(_command_buffer, &command_buffer_begin_info); err != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to begin command buffer: {}", vk_strerror(err))};
        }
        return {};
    }

    auto CommandBuffer::end() const noexcept -> kstd::Result<void> {
        if (const auto err = ::vkEndCommandBuffer(_command_buffer); err != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to begin command buffer: {}", vk_strerror(err))};
        }
        return {};
    }

    auto CommandBuffer::operator=(CommandBuffer&& other) noexcept -> CommandBuffer& {
        _command_pool = other._command_pool;
        _command_buffer = other._command_buffer;
        other._command_pool = nullptr;
        other._command_buffer = nullptr;
        return *this;
    }

    /**
     * This constructor creates a command pool on the specified device
     *
     * @param device The device for create the pool
     * @author       Cedric Hammes
     * @since        14/03/2024
     */
    CommandPool::CommandPool(const Device& device) ://NOLINT
            _device {&device},
            _command_pool {} {
        VkCommandPoolCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        create_info.queueFamilyIndex = 0; /** Add device.get_graphics_queue().get_family_index() when own queue impl **/
        if(const auto err = ::vkCreateCommandPool(*device, &create_info, nullptr, &_command_pool); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create command pool: {}", vk_strerror(err))};
        }
    }

    CommandPool::CommandPool(CommandPool&& other) noexcept :
            _device {other._device},
            _command_pool {other._command_pool} {
        other._device = nullptr;
        other._command_pool = nullptr;
    }

    /**
     * This function allocates the specified count of wrapped command buffers
     *
     * @param count The count of newly allocated command buffers
     * @return      The command buffers or an error
     * @author      Cedric Hammes
     * @since       14/03/2024
     */
    auto CommandPool::allocate(uint32_t count) const noexcept -> kstd::Result<std::vector<CommandBuffer>> {
        VkCommandBufferAllocateInfo allocate_info {};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.commandBufferCount = count;
        allocate_info.commandPool = _command_pool;

        std::vector<VkCommandBuffer> raw_command_buffers {count};
        if(const auto err = ::vkAllocateCommandBuffers(**_device, &allocate_info, raw_command_buffers.data());
           err != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to allocate {} command buffer: {}", count, vk_strerror(err))};
        }

        std::vector<CommandBuffer> command_buffers {};
        command_buffers.reserve(count);
        for(auto raw_command_buffer : raw_command_buffers) {
            command_buffers.emplace_back(this, raw_command_buffer);
        }
        return command_buffers;
    }

    /**
     * This destructor destroys the command pool when the handle is valid
     *
     * @author Cedric Hammes
     * @since  14/03/2024
     */
    CommandPool::~CommandPool() noexcept {
        if(_command_pool != nullptr) {
            ::vkDestroyCommandPool(**_device, _command_pool, nullptr);
            _command_pool = nullptr;
        }
    }

    auto CommandPool::operator=(CommandPool&& other) noexcept -> CommandPool& {
        _device = other._device;
        _command_pool = other._command_pool;
        other._device = nullptr;
        other._command_pool = nullptr;
        return *this;
    }
}// namespace libaetherium::vulkan