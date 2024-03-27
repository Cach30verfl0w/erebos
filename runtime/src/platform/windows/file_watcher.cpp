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

#ifdef PLATFORM_WINDOWS
#include "erebos/platform/file_watcher.hpp"
#include <algorithm>

namespace erebos::platform {
    namespace {
        auto mask_to_action_string(const DWORD action) noexcept -> std::string {
            if(action == FILE_ACTION_MODIFIED) {
                return "modify";
            }

            if(action == FILE_ACTION_ADDED) {
                return "create";
            }

            if(action == FILE_ACTION_REMOVED) {
                return "delete";
            }

            return fmt::format("unknown ({:X})", action);
        }

        constexpr auto action_to_event_type(const DWORD action) noexcept -> FileEventType {
            if(action == FILE_ACTION_MODIFIED) {
                return FileEventType::WRITTEN;
            }

            if(action == FILE_ACTION_ADDED) {
                return FileEventType::CREATED;
            }

            if(action == FILE_ACTION_REMOVED) {
                return FileEventType::DELETED;
            }

            return FileEventType::UNKNOWN;
        }
    }// namespace

    FileWatcher::FileWatcher(std::filesystem::path base_path)
        : _base_path {std::move(base_path)}
        , _overlapped {}
        , _is_running {true}
        , _event_queue_mutex {}
        , _event_queue {}
        , _event_buffer {} {
        _overlapped.hEvent = ::CreateEvent(nullptr, true, false, nullptr);
        _handle = ::CreateFile(_base_path.string().c_str(),
                               FILE_LIST_DIRECTORY,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               nullptr,
                               OPEN_EXISTING,
                               FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                               nullptr);
        if(_handle == invalid_file_watcher_handle) {
            throw std::runtime_error {fmt::format("Unable to create file watcher: {}", get_last_error())};
        }

        _file_watcher_thread = std::thread {[&]() {
            while(_is_running) {
                if(!::ReadDirectoryChangesW(_handle,
                                            _event_buffer.data(),
                                            _event_buffer.size(),
                                            true,
                                            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_CREATION |
                                                FILE_NOTIFY_CHANGE_SIZE,
                                            nullptr,
                                            &_overlapped,
                                            nullptr)) {
                    SPDLOG_INFO("Failed to handle file event in folder {}: {}", _base_path.string(), get_last_error());
                    continue;
                }

                const auto status = ::WaitForSingleObject(_overlapped.hEvent, 3000);
                if(status == WAIT_TIMEOUT) {
                    continue;
                }

                if(status != WAIT_OBJECT_0) {
                    SPDLOG_ERROR("Failed to handle file event in folder {} {}: {}", status, _base_path.string(), get_last_error());
                    continue;
                }

                size_t offset = 0;
                FILE_NOTIFY_INFORMATION* notify;

                do {
                    notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(_event_buffer.data() + offset);
                    std::wstring file_name;
                    file_name.assign(notify->FileName, notify->FileNameLength / sizeof(WCHAR));
                    const auto path = _base_path / erebos::utils::to_mbs(file_name);

                    {
                        const auto lock = std::lock_guard {_event_queue_mutex};
                        _event_queue.push_back(FileEvent {action_to_event_type(notify->Action), path});
                    }
                    offset += notify->NextEntryOffset;
                } while(notify->NextEntryOffset > 0);
                ::SleepEx(100, true);
            }
        }};
    }

    FileWatcher::FileWatcher(FileWatcher&& other) noexcept
        : _handle {other._handle}
        , _base_path {std::move(other._base_path)}
        , _file_watcher_thread {std::move(other._file_watcher_thread)}
        , _overlapped {other._overlapped}
        , _event_queue {std::move(other._event_queue)}
        , _event_queue_mutex {}
        , _event_buffer {other._event_buffer} {
        _handle = invalid_file_watcher_handle;
        _is_running = true;
    }

    FileWatcher::~FileWatcher() noexcept {
        if(_handle != invalid_file_watcher_handle) {
            _is_running = false;
            if(!HasOverlappedIoCompleted(&_overlapped)) {
                ::SleepEx(5, true);
            }

            ::CloseHandle(_overlapped.hEvent);
            ::CloseHandle(_handle);
            _handle = invalid_file_watcher_handle;
        }
    }

    auto FileWatcher::operator=(FileWatcher&& other) noexcept -> FileWatcher& {
        _handle = other._handle;
        _base_path = std::move(other._base_path);
        _file_watcher_thread = std::move(other._file_watcher_thread);
        _event_queue = std::move(other._event_queue);
        _overlapped = other._overlapped;
        _event_buffer = other._event_buffer;
        other._handle = invalid_file_watcher_handle;
        _is_running = true;
        return *this;
    }
}// namespace erebos::platform
#endif