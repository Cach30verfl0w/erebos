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
 * @since  18/03/2024
 */

#include "erebos/render/dxcompiler.hpp"
#include <iostream>

namespace erebos::render {

    DXCompiler::DXCompiler(const std::filesystem::path& path)
        : _library_loader {platform::LibraryLoader {path.string()}}
        , _dxc_compiler {}
        , _dxc_utils {} {
        // clang-format off
        _DxcCreateInstance =_library_loader.get_function<HRESULT, REFCLSID, REFIID, LPVOID*>("DxcCreateInstance")
                .get_or_throw();
        // clang-format on

        if(_DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_dxc_utils)) != S_OK) {
            throw std::runtime_error {"Unable to initialize DX Compiler: Failed to initialize DXC Utils"};
        }

        if(_DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_dxc_compiler)) != S_OK) {
            throw std::runtime_error {"Unable to initialize DX Compiler: Failed to initialize DX Compiler itself"};
        }
        SPDLOG_INFO("Successfully initialized DX Compiler");
    }

    auto DXCompiler::compile(const std::vector<kstd::u8>& code, VkShaderStageFlagBits shader_stage) const noexcept
        -> kstd::Result<std::vector<std::uint32_t>> {
        using namespace std::string_literals;

        LPCWSTR profile;
        if(is_flag_set<VK_SHADER_STAGE_COMPUTE_BIT>(shader_stage)) {
            profile = L"cs_6_8";
        }
        else if(is_flag_set<VK_SHADER_STAGE_VERTEX_BIT>(shader_stage)) {
            profile = L"vs_6_8";
        }
        else if(is_flag_set<VK_SHADER_STAGE_FRAGMENT_BIT>(shader_stage)) {
            profile = L"ps_6_8";
        }
        else {
            return kstd::Error {fmt::format("Unable to compile HLSL shader: Invalid shader flags {}", static_cast<uint32_t>(shader_stage))};
        }

        // clang-format off
        std::vector<LPCWSTR> args {
                {L"-fvk-use-scalar-layout", L"-fspv-target-env=vulkan1.3", L"-spirv", L"-HV", L"2021", L"-T", profile}
        };
        // clang-format on

        DxcBuffer code_buffer {.Ptr = &*code.begin(), .Size = static_cast<std::uint32_t>(code.size()), .Encoding = 0};
        DXCPointer<IDxcResult> result;
        if(_dxc_compiler->Compile(&code_buffer, args.data(), args.size(), nullptr, IID_PPV_ARGS(&result)) != S_OK) {
            return kstd::Error {fmt::format("Unable to compile HLSL shader: {}", platform::get_last_error())};
        }

        DXCPointer<IDxcBlob> output_object;
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&output_object), nullptr);
        if(!output_object) {
            return kstd::Error {"Unable to compile HLSL Shader: No valid output object provided"s};
        }

        const auto pointer = static_cast<uint32_t*>(output_object->GetBufferPointer());
        return {{pointer, pointer + (output_object->GetBufferSize() / sizeof(uint32_t))}};
    }
}// namespace erebos::render