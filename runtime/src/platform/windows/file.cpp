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
#include "erebos/platform/file.hpp"

namespace erebos::platform {
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
     * This constructor fills this class with the pointer to the memory and the size of the
     * memory.
     *
     * @param pointer The pointer to the memory
     * @param size    The size of the memory
     * @author        Cedric Hammes
     * @since         16/03/2024
     */
    FileMapping::FileMapping(kstd::u8* file_ptr, HANDLE memory_map_handle, kstd::usize size) noexcept
        : _pointer {file_ptr}
        , _size {size}
        , _memory_map_handle {memory_map_handle} {
    }

    FileMapping::FileMapping(platform::FileMapping&& other) noexcept
        : _pointer {other._pointer}
        , _size {other._size}
        , _memory_map_handle {other._memory_map_handle} {
        other._pointer = nullptr;
        other._memory_map_handle = INVALID_HANDLE_VALUE;
    }

    FileMapping::~FileMapping() noexcept {
        if(_pointer != nullptr) {
            ::UnmapViewOfFile(_pointer);
            ::CloseHandle(_memory_map_handle);
            _memory_map_handle = INVALID_HANDLE_VALUE;
            _pointer = nullptr;
        }
    }

    auto FileMapping::operator=(platform::FileMapping&& other) noexcept -> FileMapping& {
        _memory_map_handle = other._memory_map_handle;
        _pointer = other._pointer;
        other._memory_map_handle = INVALID_HANDLE_VALUE;
        other._pointer = nullptr;
        _size = other._size;
        return *this;
    }

    /**
     * This constructor opens the specified path into a file handle. If that handle is invalid, this function
     * throws a runtime error.
     *
     * @param path The path to the file
     * @author     Cedric Hammes
     * @since      15/03/2024
     */
    File::File(std::filesystem::path path, AccessMode access_mode)
        : _path {path}
        , _access {access_mode} {
        const auto exists = std::filesystem::exists(_path);
        if(!exists && _path.has_parent_path()) {
            if(const auto parent_path = _path.parent_path(); std::filesystem::exists(parent_path)) {
                std::filesystem::create_directories(parent_path);
            }
        }

        _handle = ::CreateFileW(kstd::utils::to_wcs(path.string()).data(),
                                to_access(access_mode),
                                0,
                                nullptr,
                                exists ? OPEN_EXISTING : CREATE_NEW,
                                FILE_ATTRIBUTE_NORMAL,
                                nullptr);
        if(_handle == invalid_file_handle) {
            throw std::runtime_error {fmt::format("Unable to open file '{}': {}", _path.string(), get_last_error())};
        }
    }

    File::File(File&& other) noexcept
        : _path {std::move(other._path)}
        , _access {other._access}
        , _handle {other._handle} {
        other._handle = invalid_file_handle;
    }

    File::~File() noexcept {
        if(_handle != invalid_file_handle) {
            ::CloseHandle(_handle);
            _handle = invalid_file_handle;
        }
    }

    auto File::map_into_memory() const noexcept -> kstd::Result<FileMapping> {
        const auto file_size = get_file_size();
        if(file_size.is_error()) {
            return kstd::Error {file_size.get_error()};
        }

        int flags;
        if(are_flags_set<AccessMode, AccessMode::READ, AccessMode::WRITE, AccessMode::EXECUTE>(_access)) {
            flags = PAGE_EXECUTE_READWRITE;
        }
        else if(are_flags_set<AccessMode, AccessMode::READ, AccessMode::EXECUTE>(_access)) {
            flags = PAGE_EXECUTE_READ;
        }
        else if(are_flags_set<AccessMode, AccessMode::READ, AccessMode::WRITE>(_access)) {
            flags = PAGE_READWRITE;
        }
        else if(are_flags_set<AccessMode, AccessMode::EXECUTE>(_access)) {
            flags = PAGE_EXECUTE;
        }
        else if(are_flags_set<AccessMode, AccessMode::READ>(_access)) {
            flags = PAGE_READONLY;
        }
        else {
            using namespace std::string_literals;
            return kstd::Error {"Unable to map the file into the memory: Illegal flags"s};
        }

        // Create file mapping
        const auto file_mapping_handle = ::CreateFileMapping(_handle, nullptr, flags, 0, 0, nullptr);
        if(file_mapping_handle == nullptr) {
            return kstd::Error {fmt::format("Unable to map the file into the memory: {}", platform::get_last_error())};
        }

        // Create desired access
        int desired_access = 0;
        if(are_flags_set<AccessMode, AccessMode::EXECUTE>(_access)) {
            desired_access = FILE_MAP_EXECUTE;
        }

        if(are_flags_set<AccessMode, AccessMode::WRITE>(_access)) {
            desired_access = FILE_MAP_WRITE;
        }


        if(are_flags_set<AccessMode, AccessMode::READ>(_access)) {
            desired_access = FILE_MAP_READ;
        }

        // Map view of file and return
        const auto base_ptr = ::MapViewOfFile(file_mapping_handle, desired_access, 0, 0, 0);
        if(base_ptr == nullptr) {
            CloseHandle(file_mapping_handle);
            return kstd::Error {fmt::format("{}", platform::get_last_error())};
        }

        return {{static_cast<kstd::u8*>(base_ptr), file_mapping_handle, *file_size}};
    }

    auto File::get_file_size() const noexcept -> kstd::Result<kstd::usize> {
        LARGE_INTEGER file_size {};
        if(!::GetFileSizeEx(_handle, &file_size)) {
            return kstd::Error {fmt::format("Unable to acquire size of file: {}", platform::get_last_error())};
        }
        return static_cast<kstd::usize>(file_size.QuadPart);
    }

    auto File::operator=(File&& other) noexcept -> File& {
        _path = std::move(other._path);
        _access = other._access;
        _handle = other._handle;
        return *this;
    }
}// namespace erebos::platform
#endif