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

#include "erebos/window.hpp"

namespace erebos {
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
    Window::Window(std::string_view title, uint32_t initial_width, uint32_t initial_height)
        : _event_handlers {} {
        using namespace std::string_literals;
        if(::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
            throw std::runtime_error {fmt::format("Unable to init SDL: {}", ::SDL_GetError())};
        }

        SPDLOG_INFO("Create SDL window '{}' with {}x{} pixels", title, initial_width, initial_height);
        _window_handle = ::SDL_CreateWindow(title.data(),
                                            SDL_WINDOWPOS_UNDEFINED,
                                            SDL_WINDOWPOS_UNDEFINED,
                                            static_cast<int32_t>(initial_width),
                                            static_cast<int32_t>(initial_height),
                                            SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
        if(_window_handle == nullptr) {
            throw std::runtime_error {fmt::format("Unable to init SDL window: {}", ::SDL_GetError())};
        }

        ::SDL_ShowWindow(_window_handle);
    }

    Window::Window(Window&& other) noexcept
        : _window_handle {other._window_handle}
        , _event_handlers {std::move(other._event_handlers)} {
        other._window_handle = nullptr;
    }

    /**
     * This destructor destroys the handle of the window and quits SDL if the handle is
     * valid.
     *
     * @author Cedric Hammes
     * @since  14/03/2024
     */
    Window::~Window() noexcept {
        if(_window_handle != nullptr) {
            ::SDL_DestroyWindow(_window_handle);
            ::SDL_Quit();
            _window_handle = nullptr;
        }
    }

    /**
     * This method runs a window loop which polls window events while the window doesn't requested to be closed. When
     * an error occurs, this method cancels the loop and returns the error.
     *
     * @return Void or an error
     * @author Cedric Hammes
     * @since  14/03/2024
     */
    auto Window::run_loop() const noexcept -> kstd::Result<void> {
        auto is_running = true;

        SDL_Event event {};
        while(is_running) {
            while(is_running && ::SDL_PollEvent(&event)) {
                if(event.type == SDL_QUIT) {
                    is_running = false;
                    continue;
                }

                for(const auto& handlers : _event_handlers) {
                    if(const auto result = handlers.first(event, handlers.second); result.is_error()) {
                        SPDLOG_ERROR("Error while handling SDL event -> {}", result.get_error());
                    }
                }
            }

            // TODO: Do render after event fetch
        }
        return {};
    }

    auto Window::operator=(Window&& other) noexcept -> Window& {
        _window_handle = other._window_handle;
        _event_handlers = std::move(other._event_handlers);
        other._window_handle = nullptr;
        return *this;
    }
}// namespace erebos