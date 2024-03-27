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
#include <cstring>
#include <fmt/format.h>
#include <string>

#ifdef PLATFORM_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "erebos/unicode.hpp"
#include <Windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

namespace erebos::platform {
#ifdef PLATFORM_WINDOWS
    using ModuleHandle = HMODULE;
    using FileHandle = HANDLE;
    using FileWatcherHandle = HANDLE;

    static inline const FileHandle invalid_file_handle = INVALID_HANDLE_VALUE;
    static inline const FileHandle invalid_file_watcher_handle = INVALID_HANDLE_VALUE;
#else
    using ModuleHandle = void*;
    using FileHandle = int;
    using FileWatcherHandle = int;

    static inline const FileHandle invalid_file_handle = -1;
    static inline const FileHandle invalid_file_watcher_handle = -1;
#endif

    static inline ModuleHandle invalid_module_handle = nullptr;

    [[nodiscard]] auto get_last_error() noexcept -> std::string;
}// namespace erebos::platform