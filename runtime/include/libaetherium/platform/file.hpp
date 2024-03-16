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
#include "libaetherium/platform/platform.hpp"
#include <filesystem>
#include <kstd/bitflags.hpp>
#include <kstd/defaults.hpp>
#include <kstd/types.hpp>
#include <libaetherium/utils.hpp>

#ifdef PLATFORM_LINUX
#include <fcntl.h>
#endif

namespace libaetherium::platform {
    KSTD_BITFLAGS(uint8_t, AccessMode, READ = 0b001, WRITE = 0b010, EXECUTE = 0b100)

    /**
     * This class is a cross-platform implementation over the file APIs of different operating system. This system
     * provides the functionality to load files into the memory, write and write files, etc.
     *
     * @author Cedric Hammes
     * @since  15/03/2024
     */
    class File final {
        std::filesystem::path _path;
        AccessMode _access;
        FileHandle _handle;

    public:
        /**
         * This constructor opens the specified path into a file handle. If that handle is invalid, this function
         * throws a runtime error.
         *
         * @param path The path to the file
         * @author     Cedric Hammes
         * @since      15/03/2024
         */
        File(std::filesystem::path path, AccessMode access_mode);
        File(File&& other) noexcept;
        ~File() noexcept;
        KSTD_NO_COPY(File, File);

        [[nodiscard]] auto get_file_size() const noexcept -> kstd::Result<kstd::usize>;

        [[nodiscard]] inline auto operator*() const noexcept -> FileHandle {
            return _handle;
        }

        auto operator=(File&& other) noexcept -> File&;
    };
}// namespace libaetherium::platform