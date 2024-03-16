cmake_minimum_required(VERSION 3.27)
project(editor)

file(GLOB_RECURSE EDITOR_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/editor/*.c*")
add_executable(aetherium-editor ${EDITOR_SOURCE_FILES})
target_include_directories(aetherium-editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/editor/include")

# Add fmt
target_include_directories(aetherium-editor PUBLIC "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")

# Add cxxopts
FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_MakeAvailable(cxxopts)
target_include_directories(aetherium-editor PUBLIC "${CMAKE_BINARY_DIR}/_deps/cxxopts-src/include")

# Add spdlog
target_include_directories(aetherium-editor PUBLIC "${CMAKE_BINARY_DIR}/_deps/spdlog-src/include")
add_compile_definitions(SPDLOG_HEADER_ONLY SPDLOG_FMT_EXTERNAL)

if (${BUILD_DEBUG})
    message(STATUS "Verbose logging is enabled in debug build")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
else ()
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif ()

# Add runtime as dependency
target_link_libraries(aetherium-editor PRIVATE aetherium-static)
add_dependencies(aetherium-editor aetherium-static)