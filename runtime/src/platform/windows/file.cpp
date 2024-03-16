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

#ifdef PLATFORM_WINDOWS
#include "libaetherium/platform/file.hpp"

namespace libaetherium::platform {
    namespace {
        /**
         * This function converts the specified access mode into the access flags for the
         * open function.
         *
         * @param mode The access mode specified
         * @return     The access flags of the file
         * @author     Cedric Hammes
         * @since      15/03/2024
         */
        [[nodiscard]] constexpr auto to_access(const AccessMode mode) noexcept -> DWORD {
            DWORD flags = 0;
            if(are_flags_set<AccessMode, AccessMode::WRITE>(mode)) {
                flags |= GENERIC_WRITE;
            }

            if(are_flags_set<AccessMode, AccessMode::READ>(mode)) {
                flags |= GENERIC_READ;
            }

            if(are_flags_set<AccessMode, AccessMode::READ>(mode)) {
                flags |= GENERIC_EXECUTE;
            }

            return flags;
        }
    }// namespace

    /**
     * This constructor opens the specified path into a file handle. If that handle is invalid, this function
     * throws a runtime error.
     *
     * @param path The path to the file
     * @author     Cedric Hammes
     * @since      15/03/2024
     */
    File::File(std::filesystem::path path, AccessMode access_mode) ://NOLINT
            _path {path},
            _access {access_mode} {
        const auto exists = std::filesystem::exists(_path);
        if(!exists && _path.has_parent_path()) {
            if(const auto parent_path = _path.parent_path(); std::filesystem::exists(parent_path)) {
                std::filesystem::create_directories(parent_path);
            }
        }

        _handle = ::CreateFileW(kstd::utils::to_wcs(path.string()).data(), to_access(access_mode), 0, nullptr,
                                exists ? OPEN_EXISTING : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if(_handle == invalid_file_handle) {
            throw std::runtime_error {fmt::format("Unable to open file '{}': {}", _path.string(), get_last_error())};
        }
    }

    File::File(File&& other) noexcept :
            _path {std::move(other._path)},
            _access {other._access},
            _handle {other._handle} {
        other._handle = invalid_file_handle;
    }

    File::~File() noexcept {
        if(_handle != invalid_file_handle) {
            ::CloseHandle(_handle);
            _handle = invalid_file_handle;
        }
    }

    auto File::get_file_size() const noexcept -> kstd::Result<kstd::usize> {
        DWORD file_size = 0;
        if(::GetFileSize(_handle, &file_size) == INVALID_FILE_SIZE) {
            return kstd::Error {fmt::format("Unable to acquire size of file: {}", platform::get_last_error())};
        }
        return {file_size};
    }

    auto File::operator=(File&& other) noexcept -> File& {
        _path = std::move(other._path);
        _access = other._access;
        _handle = other._handle;
        return *this;
    }
}// namespace libaetherium::platform
#endif