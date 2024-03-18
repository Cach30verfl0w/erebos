cmake_minimum_required(VERSION 3.27)
project(runtime)

file(GLOB_RECURSE RUNTIME_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/runtime/src/*.c*")


# Configure libraries
message("Configure libaetherium")
add_library(aetherium SHARED ${RUNTIME_SOURCE_FILES})
target_include_directories(aetherium PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/runtime/include")
add_library(aetherium-static STATIC ${RUNTIME_SOURCE_FILES})
target_include_directories(aetherium-static PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/runtime/include")

# Add SDL2
FetchContent_Declare(
        sdl2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG SDL2
)
FetchContent_MakeAvailable(sdl2)

target_link_libraries(aetherium PUBLIC SDL2-static)
add_dependencies(aetherium SDL2-static)
target_link_libraries(aetherium-static PUBLIC SDL2-static)
add_dependencies(aetherium-static SDL2-static)

# Add dxc
target_include_directories(aetherium PUBLIC "${CMAKE_BINARY_DIR}/_deps/dxc-src/include")
target_include_directories(aetherium-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/dxc-src/include")

# Add fmt
target_include_directories(aetherium PUBLIC "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")
target_include_directories(aetherium-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")

# Add spdlog
target_include_directories(aetherium PUBLIC "${CMAKE_BINARY_DIR}/_deps/spdlog-src/include")
target_include_directories(aetherium-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/spdlog-src/include")
add_compile_definitions(SPDLOG_HEADER_ONLY SPDLOG_FMT_EXTERNAL)

# Add kstd-core
FetchContent_Declare(
        kstd-core
        GIT_REPOSITORY https://github.com/karmakrafts/kstd-core.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_Populate(kstd-core)

target_include_directories(aetherium PUBLIC "${CMAKE_BINARY_DIR}/_deps/kstd-core-src/include")
target_include_directories(aetherium-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/kstd-core-src/include")

# Add volk
FetchContent_Declare(
        volk
        GIT_REPOSITORY https://github.com/zeux/volk.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_MakeAvailable(volk)

target_include_directories(aetherium PUBLIC "${CMAKE_BINARY_DIR}/_deps/volk-src/include")
target_link_libraries(aetherium PUBLIC volk)
target_include_directories(aetherium-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/volk-src/include")
target_link_libraries(aetherium-static PUBLIC volk)

# Add Vulkan Memory Allocator
FetchContent_Declare(
        vk_mem_alloc
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_MakeAvailable(vk_mem_alloc)

target_include_directories(aetherium PUBLIC "${CMAKE_BINARY_DIR}/_deps/vk_mem_alloc-src/include")
target_link_libraries(aetherium PUBLIC VulkanMemoryAllocator)
target_include_directories(aetherium-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/vk_mem_alloc-src/include")
target_link_libraries(aetherium-static PUBLIC VulkanMemoryAllocator)

# Add gtest to libaetherium-test
message("Configure google test for libaetherium")
FetchContent_Declare(
        google-test
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
)
FetchContent_MakeAvailable(google-test)
enable_testing(TRUE)
include(GoogleTest)

# Add tests
#file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/runtime/tests/*.c*")
#add_executable(aetherium-tests ${TEST_SOURCES})
#gtest_discover_tests(aetherium-tests)
#target_link_libraries(aetherium-tests PUBLIC gtest_main)
#target_link_libraries(aetherium-tests PRIVATE aetherium-static)
#add_dependencies(aetherium-tests aetherium-static)