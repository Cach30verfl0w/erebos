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

#pragma once
#include "libaetherium/platform/file_watcher.hpp"
#include <kstd/safe_alloc.hpp>
#include <stdexcept>

namespace libaetherium::resource {
    class Resource {
    public:
        Resource() noexcept = default;
        virtual ~Resource() noexcept = default;

        virtual auto reload(const void* data, kstd::usize size) noexcept -> kstd::Result<void> {
            return {};
        }
    };

    class CachedResource final {
        platform::File _file_handle;
        platform::FileMapping _file_content;
        std::shared_ptr<Resource> _resource;

        friend class ResourceManager;

    public:
        CachedResource(platform::File file, platform::FileMapping mapping, std::shared_ptr<Resource> resource) ://NOLINT
                _file_handle {std::move(file)},
                _file_content {std::move(mapping)},
                _resource {resource} {
        }
        KSTD_DEFAULT_MOVE(CachedResource, CachedResource);
    };

    class ResourceManager final {
        const platform::FileWatcher _file_watcher;
        std::filesystem::path _assets_folder;
        std::unordered_map<std::string, CachedResource> _loaded_resources;

    public:
        ResourceManager(const std::filesystem::path& assets_folder);
        ~ResourceManager() noexcept = default;

        template<typename RESOURCE, typename... ARGS>
        auto get_resource(const std::string path, ARGS&&... args) noexcept -> kstd::Result<RESOURCE&> {
            static_assert(std::is_base_of_v<Resource, RESOURCE>, "Resource type hasn't Resource as Base");

            // Return resource ptr when found
            const auto cached_resource = _loaded_resources.find(path);
            if(cached_resource != _loaded_resources.end()) {
                return {static_cast<RESOURCE&>(*cached_resource->second._resource)};
            }

            // Check if resource exists on file system
            const auto filesystem_path = _assets_folder / path;
            if(!std::filesystem::exists(filesystem_path)) {
                return kstd::Error {
                        fmt::format("Unable to find resource '{}': Resource not existing on filesystem", path)};
            }

            // Read resource into memory
            auto resource = std::shared_ptr<RESOURCE>(std::forward<ARGS>(args)...);

            auto file = kstd::try_construct<platform::File>(filesystem_path, platform::AccessMode::READ);
            if(!file) {
                return kstd::Error {file.get_error()};
            }

            auto mapping = file->map_into_memory();
            if(!mapping) {
                return kstd::Error {mapping.get_error()};
            }
            resource->reload(**mapping, mapping->get_size());

            // clang-format off
            _loaded_resources.insert(std::pair(std::move(path), CachedResource {
                    std::move(*file),
                    std::move(*mapping),
                    resource
            }));
            // clang-format on
            return {*resource};
        }
    };
}// namespace libaetherium::resource