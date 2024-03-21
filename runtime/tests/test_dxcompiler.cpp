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
 * @since  20/03/2024
 */

#include <gtest/gtest.h>
#include <erebos/render/dxcompiler.hpp>

TEST(erebos_render_DXCompiler, test_compile) {
#ifdef PLATFORM_UNIX
    constexpr std::string_view file_path = "./libdxcompiler.so";
#else
    constexpr std::string_view file_path = "./dxcompiler.dll";
#endif
    constexpr std::string_view code = R"(
        [numthreads(8, 8, 8)] void main(uint3 global_i : SV_DispatchThreadID) {
        }
    )";

    const auto compiler = kstd::try_construct<erebos::render::DXCompiler>(file_path);
    compiler.throw_if_error();
    ASSERT_TRUE(compiler->compile({code.cbegin(), code.cend()}, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT));
}