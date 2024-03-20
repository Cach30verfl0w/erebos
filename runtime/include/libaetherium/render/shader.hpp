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
#include "libaetherium/render/dxcompiler.hpp"
#include "libaetherium/resource/resource_manager.hpp"
#include "libaetherium/vulkan/device.hpp"
#include <kstd/option.hpp>

namespace libaetherium::render {
    class Shader final : public resource::Resource {
        const vulkan::Device* _device;
        const DXCompiler* _shader_compiler;
        kstd::Option<VkShaderModule> _shader;
        VkShaderStageFlagBits _stage;

    public:
        Shader(const vulkan::Device& device, const DXCompiler& shader_compiler, VkShaderStageFlagBits stage) noexcept;
        ~Shader() noexcept;
        KSTD_DEFAULT_MOVE(Shader, Shader);
        KSTD_NO_COPY(Shader, Shader);

        [[nodiscard]] auto reload(const kstd::u8* data, kstd::usize size) noexcept -> kstd::Result<void> override;

        [[nodiscard]] inline auto operator*() const noexcept -> const kstd::Option<VkShaderModule>& {
            return _shader;
        }
    };
}// namespace libaetherium::render