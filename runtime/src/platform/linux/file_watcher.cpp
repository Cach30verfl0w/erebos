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

#ifdef PLATFORM_LINUX
#include "libaetherium/platform/file_watcher.hpp"

namespace libaetherium::platform {
    namespace {
        auto mask_to_action_string(const auto mask) noexcept -> std::string {
            if(is_flag_set<kstd::u32, IN_DELETE_SELF | IN_DELETE, IN_MOVED_FROM>(mask)) {
                return "delete";
            }

            if(is_flag_set<kstd::u32, IN_CREATE, IN_MOVED_TO>(mask)) {
                return "create";
            }

            return fmt::format("unknown ({:X})", mask);
        }
    }// namespace

    const auto watch_mask = IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_TO | IN_MOVED_FROM;

    FileWatcher::FileWatcher(std::filesystem::path base_path) ://NOLINT
            _base_path {std::move(base_path)},
            _is_running {true},
            _handle_to_path_map {} {
        constexpr auto event_buffer_size = (sizeof(inotify_event) + NAME_MAX + 1) * 10;
        static_assert(event_buffer_size < 4096, "Buffer size should be less than 4kB");

        _handle = ::inotify_init();
        if(_handle == invalid_file_handle) {
            throw std::runtime_error {fmt::format("Unable to create file watcher: {}", get_last_error())};
        }

        if (::fcntl(_handle, F_SETFL, ::fcntl(_handle, F_GETFL, 0) | O_NONBLOCK) < 0) {
            throw std::runtime_error {fmt::format("Unable to make inotify non-blocking: {}", get_last_error())};
        }

        // Add all files and directories recursively to the file watcher
        for(const auto& path : std::filesystem::recursive_directory_iterator {_base_path}) {
            const auto watch_fd = ::inotify_add_watch(_handle, path.path().c_str(), watch_mask);
            if(watch_fd == invalid_file_watcher_handle) {
                SPDLOG_ERROR("Unable to add path '{}' to watcher: {}", path.path().c_str(), get_last_error());
                continue;
            }
            _handle_to_path_map[watch_fd] = path;
        }

        _file_watcher_thread = std::thread {[&]() {
            while(_is_running) {
                std::array<kstd::u8, event_buffer_size> buffer {};
                const auto buffer_size = ::read(_handle, buffer.data(), event_buffer_size);
                if(buffer_size <= 0) {
                    continue;
                }

                kstd::u8* current_address = buffer.data();
                while(current_address < buffer.data() + buffer_size) {
                    const auto* event = reinterpret_cast<const inotify_event*>(current_address);
                    current_address += sizeof(inotify_event) + event->len;
                    if(event->len == 0) {
                        continue;
                    }

                    const auto flags = event->mask;
                    const auto path = _handle_to_path_map[event->wd] / event->name;
                    SPDLOG_TRACE("Received {} file event about '{}'", mask_to_action_string(flags), path.string());
                    if(are_flags_set<kstd::u32, IN_MOVED_FROM>(flags)) {
                        ::inotify_rm_watch(_handle, event->wd);
                        _handle_to_path_map.erase(event->wd);
                    }

                    // TODO: Handle events for update callbacks or other stuff

                    if(are_flags_set<kstd::u32, IN_CREATE, IN_MOVED_TO>(flags)) {
                        const auto watch_fd = ::inotify_add_watch(_handle, path.c_str(), watch_mask);
                        if(watch_fd == invalid_file_watcher_handle) {
                            SPDLOG_ERROR("Unable to add path '{}' of path: {}", path.c_str(), get_last_error());
                            continue;
                        }
                        _handle_to_path_map[watch_fd] = path;
                    }

                    if(are_flags_set<kstd::u32, IN_DELETE, IN_DELETE_SELF>(flags)) {
                        ::inotify_rm_watch(_handle, event->wd);
                        _handle_to_path_map.erase(event->wd);
                    }
                }
            }
        }};
    }

    FileWatcher::~FileWatcher() noexcept {
        if(_handle != invalid_file_watcher_handle) {
            ::close(_handle);
            _is_running = false;
            _handle = invalid_file_watcher_handle;
            _file_watcher_thread.join();
        }
    }
}// namespace libaetherium::platform
#endif