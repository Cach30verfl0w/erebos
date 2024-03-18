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

#include "libaetherium/render/dxcompiler.hpp"
#include <iostream>

namespace libaetherium::render {
    DXCompiler::DXCompiler(const std::filesystem::path& path) ://NOLINT
            _library_loader {platform::LibraryLoader {path.string()}},
            _dxc_compiler {},
            _dxc_utils {} {
        // clang-format off
        _DxcCreateInstance =_library_loader.get_function<HRESULT, REFCLSID, REFIID, LPVOID*>("DxcCreateInstance")
                .get_or_throw();
        // clang-format on

        _DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_dxc_utils));
        if (!_dxc_utils) {
            throw std::runtime_error {"Unable to initialize DX Compiler: Failed to initialize DXC Utils"};
        }

        _DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_dxc_compiler));
        if (!_dxc_compiler) {
            throw std::runtime_error {"Unable to initialize DX Compiler: Failed to initialize DX Compiler itself"};
        }
        SPDLOG_INFO("Successfully initialized DX Compiler");

        std::string_view hlsl = R"(
            [numthreads(8, 8, 8)] void main(uint3 global_i : SV_DispatchThreadID) {
            }
        )";
        DxcBuffer buffer {
                .Ptr = &*hlsl.begin(),
                .Size = static_cast<std::uint32_t>(hlsl.size()),
                .Encoding = 0
        };

        std::vector<LPCWSTR> args {};
        args.push_back(L"-Zpc");
        args.push_back(L"-HV");
        args.push_back(L"2021");
        args.push_back(L"-T");
        args.push_back(L"cs_6_0");
        args.push_back(L"-E");
        args.push_back(L"main");
        args.push_back(L"-spirv");
        args.push_back(L"-fspv-target-env=vulkan1.3");
        DXCPointer<IDxcResult> result;
        _dxc_compiler->Compile(&buffer, args.data(), static_cast<uint32_t>(args.size()), nullptr, IID_PPV_ARGS(&result));

        DXCPointer<IDxcBlob> shader_obj;
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader_obj), nullptr);
        SPDLOG_INFO("{}\n", shader_obj->GetBufferSize());

        std::vector<uint32_t> spirv_buffer;
        spirv_buffer.resize(shader_obj->GetBufferSize() / sizeof(uint32_t));

        for (size_t i = 0; i < spirv_buffer.size(); i++) {
            uint32_t spv = static_cast<uint32_t*>(shader_obj->GetBufferPointer())[i];
            spirv_buffer[i] = spv;
            std::cout << std::hex << (void*) spv << " ";
            if (i % 8 == 7) {
                std::cout << std::endl;
            }
        }
    }

}// namespace libaetherium::render