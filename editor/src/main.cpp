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
#include <erebos/vulkan/context.hpp>
#include <erebos/vulkan/device.hpp>
#include <erebos/window.hpp>
#include <kstd/safe_alloc.hpp>
#include <spdlog/spdlog.h>
#include <rps/core/rps_api.h>

RPS_DECLARE_RPSL_ENTRY(main, main)

auto render(SDL_Event& event, void* pointer) -> kstd::Result<void> {
    auto* render_graph = static_cast<RpsRenderGraph*>(pointer);
    // TODO: Do stuff
    return {};
}

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
    auto window = kstd::try_construct<erebos::Window>("Aetherium Editor");
    if(!window) {
        SPDLOG_ERROR("{}", window.get_error());
        return -1;
    }

    const auto vulkan_context = kstd::try_construct<erebos::vulkan::VulkanContext>(*window);
    if(!vulkan_context) {
        SPDLOG_ERROR("{}", vulkan_context.get_error());
        return -1;
    }

    const auto device = erebos::vulkan::find_device(*vulkan_context);
    if(device.is_error()) {
        SPDLOG_ERROR("{}", device.get_error());
        return -1;
    }

    // Init render graph TODO: Remove
    RpsRpslEntry entry = RPS_ENTRY_REF(main, main);
    RpsRenderGraphCreateInfo render_graph_create_info {};
    std::array<RpsQueueFlags, 3> flags = {RPS_QUEUE_FLAG_GRAPHICS, RPS_QUEUE_FLAG_COMPUTE, RPS_QUEUE_FLAG_COPY};
    render_graph_create_info.scheduleInfo.pQueueInfos = flags.data();
    render_graph_create_info.scheduleInfo.numQueues = 1;
    render_graph_create_info.mainEntryCreateInfo.hRpslEntryPoint = entry;
    RpsRenderGraph render_graph {};
    SPDLOG_INFO("Create Render Graph");
    if (RPS_FAILED(rpsRenderGraphCreate(device->get_rps_device(), &render_graph_create_info, &render_graph))) {
        SPDLOG_ERROR("Unable to create render graph");
        return -1;
    }

    SPDLOG_INFO("Entering window event loop");
    window->add_event_handler(render, &render_graph);
    if(const auto result = window->run_loop(); result.is_error()) {
        SPDLOG_ERROR("{}", result.get_error());
        return -1;
    }
    rpsRenderGraphDestroy(render_graph);
    return 0;
}
