cmake_minimum_required(VERSION 3.27)
project(editor)

# Configure platform-specific folders
set(RPS_HLSLC_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/_deps/rps-src/tools/rps_hlslc")
if (PLATFORM_WINDOWS)
    set(RPS_TARGET_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/tools/rps_hlslc/win-x64")
    file(MAKE_DIRECTORY "${RPS_TARGET_FOLDER}")

    configure_file("${RPS_HLSLC_FOLDER}/win-x64/libdxcompiler.dll" "${RPS_TARGET_FOLDER}/libdxcompiler.dll" COPYONLY)
    configure_file("${RPS_HLSLC_FOLDER}/win-x64/libdxil.dll" "${RPS_TARGET_FOLDER}/libdxil.dll" COPYONLY)
    configure_file("${RPS_HLSLC_FOLDER}/win-x64/rps-jit.dll" "${RPS_TARGET_FOLDER}/rps-jit.dll" COPYONLY)
    configure_file("${RPS_HLSLC_FOLDER}/win-x64/llvm-cbe.exe" "${RPS_TARGET_FOLDER}/llvm-cbe.exe" COPYONLY)
    configure_file("${RPS_HLSLC_FOLDER}/win-x64/rps-hlslc.exe" "${RPS_TARGET_FOLDER}/rps-hlslc.exe" COPYONLY)
elseif (PLATFORM_LINUX)
    set(RPS_LIB_TARGET_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/tools/rps_hlslc/linux-x64/lib")
    set(RPS_BIN_TARGET_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/tools/rps_hlslc/linux-x64/bin")
    file(MAKE_DIRECTORY "${RPS_LIB_TARGET_FOLDER}")
    file(MAKE_DIRECTORY "${RPS_BIN_TARGET_FOLDER}")

    configure_file("${RPS_HLSLC_FOLDER}/linux-x64/lib/libdxcompiler.so" "${RPS_LIB_TARGET_FOLDER}/libdxcompiler.so" COPYONLY)
    configure_file("${RPS_HLSLC_FOLDER}/linux-x64/lib/libdxil.so" "${RPS_LIB_TARGET_FOLDER}/libdxil.so" COPYONLY)
    configure_file("${RPS_HLSLC_FOLDER}/linux-x64/bin/llvm-cbe" "${RPS_BIN_TARGET_FOLDER}/llvm-cbe" COPYONLY)
    configure_file("${RPS_HLSLC_FOLDER}/linux-x64/bin/rps-hlslc" "${RPS_BIN_TARGET_FOLDER}/rps-hlslc" COPYONLY)
else ()
    message(FATAL_ERROR "No support for this platform")
endif ()

# RPSL
set(RpsOsLinux ON)
set(GENERATED_SOURCES "")
CompileRpslDxc(erebos "${CMAKE_CURRENT_SOURCE_DIR}/editor/src/main.rpsl" GENERATED_SOURCES "generated")

# Project itself
file(GLOB_RECURSE EDITOR_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/editor/*.c*")
add_executable(erebos-editor ${EDITOR_SOURCE_FILES} ${GENERATED_SOURCES})
target_include_directories(erebos-editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/editor/include")

# Add fmt
target_include_directories(erebos-editor PUBLIC "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")

# Add cxxopts
FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_MakeAvailable(cxxopts)
target_include_directories(erebos-editor PUBLIC "${CMAKE_BINARY_DIR}/_deps/cxxopts-src/include")

# Add spdlog
target_include_directories(erebos-editor PUBLIC "${CMAKE_BINARY_DIR}/_deps/spdlog-src/include")
add_compile_definitions(SPDLOG_HEADER_ONLY SPDLOG_FMT_EXTERNAL)

if (${BUILD_DEBUG})
    message(STATUS "Verbose logging is enabled in debug build")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
else ()
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif ()

# Add runtime as dependency
target_link_libraries(erebos-editor PRIVATE erebos-static)
add_dependencies(erebos-editor erebos-static)
