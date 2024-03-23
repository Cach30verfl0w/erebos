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
#define RPS_VK_DYNAMIC_LOADING 1
#include <kstd/result.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <rps/rps.h>
#include <string>
#include <vector>
#include <volk.h>

namespace erebos {
    using VoidResult = kstd::Result<void>;

    template<typename T, T... FLAGS>
    [[nodiscard]] constexpr auto are_flags_set(T value) noexcept -> bool {
        constexpr auto flags = (FLAGS | ...);
        return (value & flags) == flags;
    }

    template<auto... FLAGS>
    [[nodiscard]] constexpr auto is_flag_set(auto value) noexcept -> bool {
        return (value & (FLAGS | ...)) != 0;
    }

    /**
     * This functions converts the specified vulkan error code into a human-readable string-representation of the
     * error.
     *
     * @param error_code Vulkan error code
     * @return           The error message
     * @author           Cedric Hammes
     * @since            14/03/2024
     */
    [[nodiscard]] constexpr auto vk_strerror(const VkResult error_code) noexcept -> std::string {
        switch(error_code) {
            case VK_SUCCESS:
                return "Success";
            case VK_NOT_READY:
                return "Not ready";
            case VK_TIMEOUT:
                return "Timed out";
            case VK_INCOMPLETE:
                return "Incomplete";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                return "Out of device memory";
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                return "Out of host memory";
            case VK_ERROR_MEMORY_MAP_FAILED:
                return "Memory map failed";
            case VK_ERROR_LAYER_NOT_PRESENT:
                return "Layer not present";
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "Extension not present";
            case VK_ERROR_FEATURE_NOT_PRESENT:
                return "Feature not present";
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                return "Incompatible driver";
            case VK_ERROR_TOO_MANY_OBJECTS:
                return "Too many objects";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                return "Format not supported";
            case VK_ERROR_FRAGMENTED_POOL:
                return "Fragmented pool";
            case VK_ERROR_SURFACE_LOST_KHR:
                return "Surface lost";
            case VK_PIPELINE_COMPILE_REQUIRED:
                return "Pipeline compile required";
            case VK_ERROR_INVALID_SHADER_NV:
                return "Invalid shader";
            case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
                return "Invalid device address";
            default:
                return fmt::format("Unknown ({})", static_cast<int8_t>(error_code));
        }
    }

    [[nodiscard]] constexpr auto rps_strerror(RpsResult error_code) noexcept -> std::string {
        switch (error_code) {
            case RPS_ERROR_INVALID_ARGUMENTS:
                return "Invalid arguments";
            case RPS_ERROR_INVALID_DATA:
                return "Invalid data";
            case RPS_ERROR_OUT_OF_MEMORY:
                return "Out of memory";
            case RPS_ERROR_INVALID_FILE_FORMAT:
                return "Invalid file format";
            case RPS_ERROR_FILE_NOT_FOUND:
                return "File not found";
            case RPS_ERROR_INVALID_OPERATION:
                return "Invalid operation";
            case RPS_ERROR_UNRECOGNIZED_COMMAND:
                return "Unrecognized command";
            case RPS_ERROR_UNSUPPORTED_VERSION_TOO_OLD:
                return "File version too old";
            case RPS_ERROR_UNSUPPORTED_VERSION_TOO_NEW:
                return "File version too new";
            case RPS_ERROR_UNKNOWN_NODE:
                return "Unknown node";
            case RPS_ERROR_INDEX_OUT_OF_BOUNDS:
                return "Index out of bounds";
            case RPS_ERROR_COMMAND_ALREADY_FINAL:
                return "Command already final";
            case RPS_ERROR_INTEROP_DATA_LAYOUT_MISMATCH:
                return "Data Layout mismatch between RUntime and Shader";
            case RPS_ERROR_KEY_NOT_FOUND:
                return "Key not found";
            case RPS_ERROR_KEY_DUPLICATED:
                return "Key duplicated";
            case RPS_ERROR_NOT_IMPLEMENTED:
                return "Not implemented";
            case RPS_ERROR_INTEGER_OVERFLOW:
                return "Integer Overflow";
            case RPS_ERROR_RANGE_OVERLAPPING:
                return "Exclusive ranges overlapping";
            case RPS_ERROR_INVALID_PROGRAM:
                return "Invalid RPSL shader program being formed";
            case RPS_ERROR_VALIDATION_FAILED:
                return "Invalid pipeline configuration";
            case RPS_ERROR_TYPE_MISMATCH:
                return "Type mismatch";
            case RPS_ERROR_NOT_SUPPORTED:
                return "Unsupported feature";
            case RPS_ERROR_INTERNAL_ERROR:
                return "RPS library internal error";
            default:
                return fmt::format("Unknown ({})", static_cast<int8_t>(error_code));
        }
    }
}// namespace erebos
