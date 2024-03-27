cmake_minimum_required(VERSION 3.27)
project(runtime)

file(GLOB_RECURSE RUNTIME_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/runtime/src/*.c*")

option(EREBOS_RUNTIME_BUILD_TESTS "Compile Tests of Erebos Runtime" ON)

# RPS
FetchContent_Declare(
        rps
        GIT_REPOSITORY https://github.com/projectKML/renderpipelineshaders
        GIT_TAG main
        GIT_PROGRESS true
)
set(RpsBuildExamples OFF)
set(RpsBuildTests OFF)
set(RpsEnableImGui OFF)
FetchContent_MakeAvailable(rps)

# Compile RPSL files
include(render-pipeline-shaders)
compile_rpsl(erebos "${CMAKE_CURRENT_SOURCE_DIR}/runtime/src/runtime.rpsl" erebos_RPSL_OUTPUT_FILE)

# Configure libraries
message("Configure liberebos")
add_library(erebos SHARED ${RUNTIME_SOURCE_FILES} ${erebos_RPSL_OUTPUT_FILE})
target_include_directories(erebos PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/runtime/include")
add_library(erebos-static STATIC ${RUNTIME_SOURCE_FILES} ${erebos_RPSL_OUTPUT_FILE})
target_include_directories(erebos-static PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/runtime/include")

# Add SDL2
FetchContent_Declare(
        sdl2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG SDL2
)
FetchContent_MakeAvailable(sdl2)

target_link_libraries(erebos PUBLIC SDL2-static)
add_dependencies(erebos SDL2-static)
target_link_libraries(erebos-static PUBLIC SDL2-static)
add_dependencies(erebos-static SDL2-static)

# Add dxc
target_include_directories(erebos PUBLIC "${CMAKE_BINARY_DIR}/_deps/dxc-src/include")
target_include_directories(erebos-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/dxc-src/include")

# Add mimalloc
FetchContent_Declare(
        mimalloc
        GIT_REPOSITORY https://github.com/Microsoft/mimalloc
        GIT_TAG v2.1.2
        GIT_PROGRESS ture
)
set(MI_OVERRIDE OFF)
FetchContent_MakeAvailable(mimalloc)
target_include_directories(erebos PUBLIC "${CMAKE_BINARY_DIR}/_deps/mimalloc-src/include")
target_include_directories(erebos-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/mimalloc-src/include")
target_link_libraries(erebos PUBLIC mimalloc-static)
target_link_libraries(erebos-static PUBLIC mimalloc-static)

# Add spirv-reflect
FetchContent_Declare(
        spirv-reflect
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect
        GIT_TAG main
        GIT_PROGRESS true
)
FetchContent_Populate(spirv-reflect)
target_include_directories(erebos PUBLIC "${CMAKE_BINARY_DIR}/_deps/spirv-reflect-src/include")
target_include_directories(erebos-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/spirv-reflect-src/include")

# Add fmt
target_include_directories(erebos PUBLIC "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")
target_include_directories(erebos-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")

# Add spdlog
target_include_directories(erebos PUBLIC "${CMAKE_BINARY_DIR}/_deps/spdlog-src/include")
target_include_directories(erebos-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/spdlog-src/include")
add_compile_definitions(SPDLOG_HEADER_ONLY SPDLOG_FMT_EXTERNAL)

# Add volk
FetchContent_Declare(
        volk
        GIT_REPOSITORY https://github.com/zeux/volk.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_MakeAvailable(volk)

target_link_libraries(erebos PUBLIC volk)
target_link_libraries(erebos-static PUBLIC volk)

# Add Vulkan Memory Allocator
FetchContent_Declare(
        vk_mem_alloc
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_MakeAvailable(vk_mem_alloc)

target_link_libraries(erebos PUBLIC VulkanMemoryAllocator)
target_link_libraries(erebos-static PUBLIC VulkanMemoryAllocator)
add_compile_definitions(VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=0)

# Add RenderPipelineShaders
target_include_directories(erebos PUBLIC "${CMAKE_BINARY_DIR}/_deps/rps-src/include")
target_include_directories(erebos-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/rps-src/include")
target_link_libraries(erebos PUBLIC rps_runtime_vkdyn)
target_link_libraries(erebos-static PUBLIC rps_runtime_vkdyn)

# Add gtest to liberebos-test
FetchContent_Declare(
        google-test
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
)
FetchContent_MakeAvailable(google-test)
enable_testing(TRUE)
include(GoogleTest)

# Add tests
if (EREBOS_RUNTIME_BUILD_TESTS)
    file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/runtime/tests/*.c*")
    add_executable(erebos-tests ${TEST_SOURCES})
    gtest_discover_tests(erebos-tests)
    target_link_libraries(erebos-tests PUBLIC gtest_main)
    target_link_libraries(erebos-tests PUBLIC erebos-static)
endif ()
