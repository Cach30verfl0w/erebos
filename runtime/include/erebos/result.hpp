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
 * @since  27/03/2024
 */

#pragma once
#include "erebos/utils.hpp"
#include <cassert>
#include <string>
#include <variant>

namespace erebos {
    template<typename T>
    class Error final {
        using value_type = T;

    private:
        T _value;

    public:
        // clang-format off
        constexpr explicit Error(T value) : _value {std::forward<T>(value)} {}
        ~Error() noexcept = default;
        // clang-format on
        EREBOS_DEFAULT_MOVE(Error);
        EREBOS_DELETE_COPY(Error);

        [[nodiscard]] constexpr auto get() const noexcept -> const T& {
            return _value;
        }

        [[nodiscard]] constexpr auto get() noexcept -> T& {
            return _value;
        }
    };

    template<typename T, typename TError = std::string>
    class Result final {
    public:
        using value_type = T;
        using error_type = TError;
        using error_type_wrapper = Error<TError>;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;

    private:
        static_assert(!std::is_same_v<std::remove_all_extents_t<value_type>, error_type>, "Type and error cannot be the same");
        std::variant<value_type, error_type_wrapper> _value_or_error;

    public:
        // clang-format off
        constexpr Result(value_type value) : _value_or_error(std::forward<value_type>(value)) {}
        constexpr Result(Error<TError> error) : _value_or_error(std::move(error)) {}
        ~Result() noexcept = default;
        // clang-format on
        EREBOS_DEFAULT_MOVE(Result);
        EREBOS_DELETE_COPY(Result);

        template<typename TNewValue>
        [[nodiscard]] constexpr auto forward() noexcept -> Result<TNewValue, TError> {
            assert(is_error());
            return std::move(std::get<error_type_wrapper>(_value_or_error));
        }

        template<typename TNewValue, typename F = std::function<TNewValue(T)>>
        [[nodiscard]] constexpr auto map(F&& function) noexcept -> Result<TNewValue, TError> {
            return is_ok() ? Result(std::forward(function(get()))) : Result(error_type_wrapper(get_error()));
        }

        [[nodiscard]] constexpr auto move_or_throw() const -> T {
            if(is_error()) {
                throw std::runtime_error(std::string(get_error()));
            }
            return std::move(std::get<value_type>(_value_or_error));
        }

        [[nodiscard]] constexpr auto get() const noexcept -> const_reference {
            assert(is_ok());
            return std::get<value_type>(_value_or_error);
        }

        [[nodiscard]] constexpr auto get() noexcept -> value_type& {
            assert(is_ok());
            return std::get<value_type>(_value_or_error);
        }

        [[nodiscard]] constexpr auto is_ok() const noexcept -> bool {
            return std::holds_alternative<value_type>(_value_or_error);
        }

        [[nodiscard]] constexpr auto is_error() const noexcept -> bool {
            return !is_ok();
        }

        [[nodiscard]] auto get_error() const noexcept -> const TError& {
            assert(!is_ok());
            return std::get<error_type_wrapper>(_value_or_error).get();
        }

        [[nodiscard]] auto get_error() noexcept -> TError& {
            assert(!is_ok());
            return std::get<error_type_wrapper>(_value_or_error).get();
        }

        [[nodiscard]] inline auto operator->() const noexcept -> const_pointer {
            assert(is_ok());
            return &std::get<value_type>(_value_or_error);
        }

        [[nodiscard]] inline auto operator->() noexcept -> pointer {
            assert(is_ok());
            return &std::get<value_type>(_value_or_error);
        }

        [[nodiscard]] inline auto operator*() const noexcept -> const_reference {
            assert(is_ok());
            return std::get<value_type>(_value_or_error);
        }

        [[nodiscard]] inline auto operator*() noexcept -> reference {
            assert(is_ok());
            return std::get<value_type>(_value_or_error);
        }

        [[nodiscard]] operator bool() const noexcept {
            return is_ok();
        }
    };

    template<typename TError>
    class Result<void, TError> final {
    public:
        using value_type = std::monostate;
        using error_type = TError;
        using error_type_wrapper = Error<TError>;

    private:
        static_assert(!std::is_same_v<std::remove_all_extents_t<value_type>, error_type>, "Type and error cannot be the same");
        std::variant<value_type, error_type_wrapper> _value_or_error;

    public:
        // clang-format off
        constexpr Result() : _value_or_error() {}
        constexpr Result(Error<TError> error) : _value_or_error(std::move(error)) {}
        ~Result() noexcept = default;
        // clang-format on
        EREBOS_DEFAULT_MOVE(Result);
        EREBOS_DELETE_COPY(Result);

        [[nodiscard]] auto get_error() const noexcept -> const TError& {
            assert(!is_ok());
            return std::get<error_type_wrapper>(_value_or_error).get();
        }

        [[nodiscard]] auto get_error() noexcept -> TError& {
            assert(!is_ok());
            return std::get<error_type_wrapper>(_value_or_error).get();
        }

        [[nodiscard]] constexpr auto is_ok() const noexcept -> bool {
            return std::holds_alternative<value_type>(_value_or_error);
        }

        [[nodiscard]] constexpr auto is_error() const noexcept -> bool {
            return !is_ok();
        }

        [[nodiscard]] operator bool() const noexcept {
            return is_ok();
        }
    };

    template<typename TValue, typename... TArgs>
    [[nodiscard]] auto try_construct(TArgs&&... args) -> Result<TValue> {
        try {
            return TValue(std::forward<TArgs>(args)...);
        }
        catch(const std::runtime_error& error) {
            return Error(std::string(error.what()));
        }
    }
}// namespace erebos