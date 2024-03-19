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
 * @since  17/03/2024
 */

#include "libaetherium/resource/resource_manager.hpp"

namespace libaetherium::resource {
    ResourceManager::ResourceManager(const std::filesystem::path& assets_folder) ://NOLINT
            _file_watcher {assets_folder},
            _assets_folder {assets_folder} {
        if(!std::filesystem::exists(_assets_folder)) {
            throw std::runtime_error {fmt::format("Unable to initialize resource manager: Folder '{}' doesn't exists",
                                                  _assets_folder.string())};
        }
        SPDLOG_INFO("Initializing resource manager in directory '{}'", _assets_folder.string());
    }

    auto ResourceManager::reload_if_necessary() noexcept -> kstd::Result<void> {
        _file_watcher.handle_event_queue([&](const platform::FileEvent& event) noexcept -> kstd::Result<void> {
            switch(event.type) {
                case platform::DELETED:
                    if(_loaded_resources.find(event.file.string()) == _loaded_resources.cend()) {
                        break;
                    }

                    _loaded_resources.erase(event.file.string());
                    break;
                case platform::WRITTEN: {
                    SPDLOG_DEBUG("Reloading file '{}'...", event.file.string());
                    const auto resource = _loaded_resources.find(event.file.string());
                    if(resource == _loaded_resources.cend()) {
                        break;
                    }

                    const auto file = kstd::try_construct<platform::File>(event.file, platform::AccessMode::READ);
                    // Fix for multiple reloads: While file attribute changes are happening, the file is locked by
                    // Windows, so the handle is invalid and I can filter that operation. Errors like file not existing
                    // are not possible. I don't really like this solution, but it works.
                    if(!file) {
                        break;
                    }

                    const auto memory_map = file->map_into_memory();
                    if(!memory_map) {
                        SPDLOG_WARN("Fail while reload resources -> {}", memory_map.get_error());
                        break;
                    }

                    const auto reload_result = resource->second->reload(**memory_map, memory_map->get_size());
                    if (!reload_result) {
                        SPDLOG_ERROR("Fail while reload resources -> {}", reload_result.get_error());
                        break;
                    }
                    break;
                }
                default: break;
            }
            return {};
        });
        return {};
    }
}// namespace libaetherium::resource