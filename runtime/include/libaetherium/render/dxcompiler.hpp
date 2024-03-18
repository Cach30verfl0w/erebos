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

#pragma once
#include "libaetherium/platform/dynlib.hpp"
#include <filesystem>
#include <kstd/safe_alloc.hpp>
#include <spdlog/spdlog.h>

// Include unknown when on Windows
#ifdef PLATFORM_WINDOWS
#include <Unknwn.h>
#include <wrl/client.h>
#endif

// Emulate UUIDs if not on MSVC
#ifndef COMPILER_MSVC
#define __EMULATE_UUID 1
#endif

#include <dxc/dxcapi.h>

namespace libaetherium::render {
#ifdef PLATFORM_WINDOWS
    template<typename T>
    using DXCPointer = Microsoft::WRL::ComPtr<T>;
#else
    template<typename T>
    using DXCPointer = CComPtr<T>;
#endif

    typedef HRESULT (__stdcall *PFN_DxcCreateInstance)(
            _In_ REFCLSID   rclsid,
            _In_ REFIID     riid,
            _Out_ LPVOID*   ppv
    );

    class DXCompiler {
        platform::LibraryLoader _library_loader;
        PFN_DxcCreateInstance _DxcCreateInstance;
        DXCPointer<IDxcUtils> _dxc_utils;
        DXCPointer<IDxcCompiler3> _dxc_compiler;

    public:
        explicit DXCompiler(const std::filesystem::path& path);
    };
}// namespace libaetherium::render