//  Copyright 2024 Cach30verfl0w
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

/**
 * @author Cedric Hammes
 * @since  14/03/2024
 */

#pragma once
#include "erebos/result.hpp"
#include "erebos/utils.hpp"
#include <SDL2/SDL.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

#ifndef SDL2_ALLOW_MAIN_MACRO
#undef main
#endif

namespace erebos {
    using EventCallbackFunction = std::function<erebos::Result<void>(SDL_Event& event, void* data)>;
    using RenderCallbackFunction = std::function<erebos::Result<void>(void* data)>;

    class Window final {
        SDL_Window* _window_handle;
        std::vector<std::pair<EventCallbackFunction, void*>> _event_callback_list;
        std::vector<std::pair<RenderCallbackFunction, void*>> _render_callback_list;

    public:
        /**
         * This constructor initializes SDL and creates the window with the specified title and the initial
         * bounds.
         *
         * @param title          The title of the window
         * @param initial_width  The initial width of the window
         * @param initial_height The initial height of the window
         * @author               Cedric Hammes
         * @since                14/03/2024
         */
        explicit Window(std::string_view title, uint32_t initial_width = 800, uint32_t initial_height = 600);
        Window(Window&& other) noexcept;
        EREBOS_DELETE_COPY(Window);

        /**
         * This destructor destroys the handle of the window and quits
         * SQL.
         *
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        ~Window() noexcept;

        inline auto add_event_callback(const EventCallbackFunction& callback_function, void* data_ptr) noexcept -> void {
            _event_callback_list.emplace_back(callback_function, data_ptr);
        }

        inline auto add_render_callback(const RenderCallbackFunction& callback_function, void* data_ptr) noexcept -> void {
            _render_callback_list.emplace_back(callback_function, data_ptr);
        }

        /**
         * This method runs a window loop which polls window events while the window doesn't requested to be
         * closed. When an error occurs, this method cancels the loop and returns the error.
         *
         * @return Void or an error
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] auto run_loop() const noexcept -> erebos::Result<void>;

        /**
         * This method returns the raw handle to the internal usd SDL
         * window.
         *
         * @return The window handle
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto operator*() const noexcept -> SDL_Window* {
            return _window_handle;
        }

        auto operator=(Window&& other) noexcept -> Window&;
    };
}// namespace erebos