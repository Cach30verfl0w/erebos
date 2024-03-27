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
 * @author Alexander Hinze
 * @since  06/05/2023
 * @see    https://github.com/karmakrafts/kstd-platform/blob/master/src/windows/dynamic_lib.cpp
 */

#ifdef PLATFORM_WINDOWS
#include "erebos/platform/dynlib.hpp"

namespace erebos::platform {
    LibraryLoader::LibraryLoader(std::string name)
        : _name {std::move(name)} {
        _handle = ::LoadLibraryW(erebos::unicode::to_wcs(_name).data());
        if(_handle == invalid_module_handle) {
            throw std::runtime_error {fmt::format("Unable to open library '{}': {}", _name, get_last_error())};
        }
    }

    LibraryLoader::LibraryLoader(LibraryLoader&& other) noexcept
        : _name {std::move(other._name)}
        , _handle {other._handle} {
        other._handle = invalid_module_handle;
    }

    LibraryLoader::~LibraryLoader() noexcept {
        if(_handle != invalid_module_handle) {
            ::FreeLibrary(_handle);
            _handle = invalid_module_handle;
        }
    }

    auto LibraryLoader::operator=(LibraryLoader&& other) noexcept -> LibraryLoader& {
        _name = std::move(other._name);
        _handle = other._handle;
        other._handle = invalid_module_handle;
        return *this;
    }

    auto LibraryLoader::get_function_address(const std::string& name) noexcept -> erebos::Result<void*> {
        auto* address = ::GetProcAddress(_handle, name.data());
        if(address == nullptr) {
            return erebos::Error {fmt::format("Could not resolve function {} in {}: {}", name, _name, get_last_error())};
        }
        return reinterpret_cast<void*>(address);
    }
}// namespace erebos::platform
#endif