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
 * @author Alexander Hinze
 * @since  06/05/2023
 * @see    https://github.com/karmakrafts/kstd-platform/blob/master/include/kstd/platform/dynamic_lib.hpp
 */

#pragma once
#include "erebos/platform/platform.hpp"
#include "erebos/unicode.hpp"
#include "erebos/result.hpp"
#include "erebos/utils.hpp"
#include <stdexcept>

namespace erebos::platform {
    class LibraryLoader final {
        std::string _name;
        ModuleHandle _handle;

    public:
        LibraryLoader()
            ://NOLINT
            _name {}
            , _handle {invalid_module_handle} {
        }

        /**
         * This constructor opens the specified library and saves a handle. When that handle is invalid, we throw a
         * runtime error.
         *
         * @param name Name of the library
         * @author Alexander Hinze
         * @since  06/05/2023
         */
        explicit LibraryLoader(std::string name);
        LibraryLoader(LibraryLoader&& other) noexcept;
        ~LibraryLoader() noexcept;
        EREBOS_DELETE_COPY(LibraryLoader);

        /**
         * This function acquires the address of the specified function (by name) and casts that address into the
         * specified function pointer.
         *
         * @tparam R    The function's return type
         * @tparam ARGS The function's argument types
         * @param name  The function's name
         * @return      Function pointer or error
         * @author Alexander Hinze
         * @since  06/05/2023
         */
        template<typename R, typename... ARGS>
        [[nodiscard]] inline auto get_function(const std::string& name) noexcept -> erebos::Result<R (*)(ARGS...)> {
            auto address_result = get_function_address(name);

            if(!address_result) {
                return address_result.forward<R (*)(ARGS...)>();
            }

            return reinterpret_cast<R (*)(ARGS...)>(*address_result);// NOLINT
        }

        [[nodiscard]] inline auto get_name() const noexcept -> const std::string& {
            return _name;
        }

        [[nodiscard]] inline auto is_loaded() const noexcept -> bool {
            return _handle != invalid_module_handle;
        }

        [[nodiscard]] inline auto get_handle() const noexcept -> ModuleHandle {
            return _handle;
        }

        auto operator=(LibraryLoader&& other) noexcept -> LibraryLoader&;

    private:
        [[nodiscard]] auto get_function_address(const std::string& name) noexcept -> erebos::Result<void*>;
    };
}// namespace erebos::platform