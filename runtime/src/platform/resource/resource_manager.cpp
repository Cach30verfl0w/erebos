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
            _assets_folder {std::move(assets_folder)} {
        if(!std::filesystem::exists(_assets_folder)) {
            throw std::runtime_error {fmt::format("Unable to initialize resource manager: Folder '{}' doesn't exists",
                                                  _assets_folder.string())};
        }
        SPDLOG_INFO("Initializing resource manager in directory '{}'", _assets_folder.string());
    }
}// namespace libaetherium::resource