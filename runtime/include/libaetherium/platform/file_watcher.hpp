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
 * @since  16/03/2024
 */

#pragma once
#include "libaetherium/platform/platform.hpp"
#include "libaetherium/utils.hpp"
#include <filesystem>
#include <kstd/types.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>

#ifdef PLATFORM_WINDOWS
#else
#include <sys/inotify.h>
#include <signal.h>
#endif

namespace libaetherium::platform {
    class FileWatcher {
        FileWatcherHandle _handle;
        std::filesystem::path _base_path;
        std::thread _file_watcher_thread;
        kstd::atomic_bool _is_running;
#ifdef PLATFORM_WINDOWS
        ::OVERLAPPED _overlapped;
        std::array<kstd::u8, 32 * 1024> _event_buffer;
#else
        std::unordered_map<FileHandle, std::filesystem::path> _handle_to_path_map;
#endif

    public:
        explicit FileWatcher(std::filesystem::path base_path);
        ~FileWatcher() noexcept;
        KSTD_NO_MOVE_COPY(FileWatcher, FileWatcher); // TODO: Allow move
    };
}// namespace libaetherium::platform