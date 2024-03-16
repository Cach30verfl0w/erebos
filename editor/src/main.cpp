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
 * @since  14/03/2024
 */

#include <cxxopts.hpp>
#include <libaetherium/vulkan/context.hpp>
#include <libaetherium/vulkan/device.hpp>
#include <libaetherium/window.hpp>
#include <spdlog/spdlog.h>

auto main(int argc, char* argv[]) -> int {
    cxxopts::Options options {"aetherium-editor"};
    options.add_option("general", cxxopts::Option {"h,help", "Get help", cxxopts::value<bool>()});
    options.add_option("general", cxxopts::Option {"v,verbose", "Enable verbose logging", cxxopts::value<bool>()});

    const auto parse_result = options.parse(argc, argv);
    spdlog::set_level(parse_result.count("verbose") ? spdlog::level::trace : spdlog::level::info);
    if(parse_result.count("help")) {
        std::string line {};
        std::stringstream help_message {options.help()};
        while(std::getline(help_message, line, '\n')) {
            SPDLOG_INFO("{}", line);
        }
        return 0;
    }

    // Create window, vulkan context and device
    const auto window = libaetherium::Window {"Aetherium Editor"};
    const auto vulkan_context = libaetherium::vulkan::VulkanContext {window};
    const auto device = libaetherium::vulkan::find_device(vulkan_context);
    if(device.is_error()) {
        SPDLOG_ERROR("{}", device.get_error());
        return -1;
    }

    SPDLOG_INFO("Entering window event loop");
    if(const auto result = window.run_loop(); result.is_error()) {
        SPDLOG_ERROR("{}", result.get_error());
        return -1;
    }
    return 0;
}