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
#include "libaetherium/platform/file.hpp"
#include "libaetherium/platform/platform.hpp"
#include "libaetherium/utils.hpp"
#include <filesystem>
#include <kstd/bitflags.hpp>
#include <kstd/types.hpp>
#include <queue>
#include <spdlog/spdlog.h>
#include <unordered_map>

#ifndef PLATFORM_WINDOWS
#include <signal.h>
#include <sys/inotify.h>
#endif

namespace libaetherium::platform {
    enum FileEventType : kstd::u8 {
        CREATED,
        DELETED,
        WRITTEN,
        UNKNOWN
    };

    struct FileEvent final {
        FileEventType type;
        std::filesystem::path file;
    };

    class FileWatcher {
        FileWatcherHandle _handle;
        std::filesystem::path _base_path;
        std::thread _file_watcher_thread;
        kstd::atomic_bool _is_running;
        std::mutex _event_queue_mutex;
        std::deque<FileEvent> _event_queue;

#ifdef PLATFORM_WINDOWS
        ::OVERLAPPED _overlapped;
        std::array<kstd::u8, 32 * 1024> _event_buffer;
#else
        std::unordered_map<FileHandle, std::filesystem::path> _handle_to_path_map;
#endif

    public:
        explicit FileWatcher(std::filesystem::path base_path);
        FileWatcher(FileWatcher&& other) noexcept;
        ~FileWatcher() noexcept;
        KSTD_NO_COPY(FileWatcher, FileWatcher);

        template<typename F>
        auto handle_event_queue(F&& callback_function) noexcept -> kstd::Result<void> {
            static_assert(std::is_convertible_v<F, std::function<kstd::Result<void>(const FileEvent&)>>,
                          "Invalid callback function");
            const auto guard = std::lock_guard {_event_queue_mutex};
            while(!_event_queue.empty()) {
                if(const auto result = callback_function(std::move(_event_queue.front())); !result) {
                    return kstd::Error {result.get_error()};
                }
                _event_queue.pop_back();
            }
            return {};
        }

        auto operator=(FileWatcher&& other) noexcept -> FileWatcher&;
    };
}// namespace libaetherium::platform