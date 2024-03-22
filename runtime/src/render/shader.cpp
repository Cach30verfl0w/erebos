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
 * @since  19/03/2024
 */

#include "erebos/render/shader.hpp"

namespace erebos::render {
    Shader::Shader(const vulkan::Device& device, const DXCompiler& compiler, VkShaderStageFlagBits stage) noexcept
        : Resource {}
        , _device {&device}
        , _shader_compiler {&compiler}
        , _shader {}
        , _stage {stage} {
    }

    Shader::~Shader() noexcept {
        if(_device != nullptr && _shader) {
            vkDestroyShaderModule(**_device, *_shader, nullptr);
        }
    }

    auto Shader::reload(const kstd::u8* data, kstd::usize size) noexcept -> kstd::Result<void> {
        if(_device == nullptr) {
            return {};
        }

        const std::vector<kstd::u8> data_vector = {data, data + size + 1};
        const auto shader_result = _shader_compiler->compile(data_vector, _stage);
        if(!shader_result) {
            return kstd::Error {shader_result.get_error()};
        }

        if(_shader) {
            vkDestroyShaderModule(**_device, *_shader, nullptr);
        }

        VkShaderModuleCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = shader_result->size() * sizeof(std::uint32_t);
        create_info.pCode = shader_result->data();
        VkShaderModule shader {};
        if(const auto error = vkCreateShaderModule(**_device, &create_info, nullptr, &shader); error != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to reload shader: {}", vk_strerror(error))};
        }
        _shader = {shader};
        return {};
    }
}// namespace erebos::render