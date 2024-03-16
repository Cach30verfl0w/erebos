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

#ifdef PLATFORM_LINUX
#include "libaetherium/platform/file.hpp"

#ifdef CPU_64_BIT
#define OPEN ::open64
#else
#define OPEN ::open
#endif

namespace libaetherium::platform {
    namespace {
        /**
         * This function converts the specified access mode into the permission for the
         * open function.
         *
         * @param mode The access mode specified
         * @return     The permissions of the file
         * @author     Cedric Hammes
         * @since      15/03/2024
         */
        [[nodiscard]] constexpr auto to_permissions(const AccessMode mode) noexcept -> int32_t {
            int32_t permissions = 0;
            if(are_flags_set<AccessMode, AccessMode::WRITE>(mode)) {
                permissions |= S_IWUSR | S_IWGRP | S_IWOTH;
            }

            if(are_flags_set<AccessMode, AccessMode::READ>(mode)) {
                permissions |= S_IRUSR | S_IRGRP | S_IROTH;
            }

            if(are_flags_set<AccessMode, AccessMode::EXECUTE>(mode)) {
                permissions |= S_IXUSR | S_IXGRP | S_IXOTH;
            }

            return permissions;
        }

        /**
         * This function converts the specified access mode into the access flags for the
         * open function.
         *
         * @param mode The access mode specified
         * @return     The access flags of the file
         * @author     Cedric Hammes
         * @since      15/03/2024
         */
        [[nodiscard]] constexpr auto to_access(const AccessMode mode) noexcept -> int32_t {
            if(are_flags_set<AccessMode, AccessMode::WRITE, AccessMode::READ>(mode)) {
                return O_CREAT | O_RDWR;
            }

            if(are_flags_set<AccessMode, AccessMode::WRITE>(mode)) {
                return O_CREAT | O_WRONLY;
            }

            if(are_flags_set<AccessMode, AccessMode::READ>(mode)) {
                return O_CREAT | O_RDONLY;
            }

            return O_CREAT;
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
    File::File(std::filesystem::path path, AccessMode access_mode) :// NOLINT
            _path {std::move(path)},
            _access {access_mode} {
        const auto exists = std::filesystem::exists(_path);
        if (!exists && _path.has_parent_path()) {
            if (const auto parent_path = _path.parent_path(); std::filesystem::exists(parent_path)) {
                std::filesystem::create_directories(parent_path);
            }
        }

        _handle = OPEN(_path.c_str(), to_access(access_mode), to_permissions(access_mode));
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
            ::close(_handle);
            _handle = invalid_file_handle;
        }
    }

    auto File::get_file_size() const noexcept -> kstd::Result<kstd::usize> {
        const auto temp_file_handle = ::fdopen(_handle, "r");
        if(temp_file_handle == nullptr) {
            return kstd::Error {fmt::format("Unable to acquire size of file: {}", platform::get_last_error())};
        }

        fseek(temp_file_handle, EOF, SEEK_END);
        return ftell(temp_file_handle);
    }

    auto File::operator=(File&& other) noexcept -> File& {
        _path = std::move(other._path);
        _access = other._access;
        _handle = other._handle;
        return *this;
    }
}// namespace libaetherium::platform
#endif