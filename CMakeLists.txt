cmake_minimum_required(VERSION 3.27)
project(aetherium)

# Default C++ and C standard
if (NOT DEFINED CMAKE_CXX_STANDARD)
    message(STATUS "No C++ version specified, defaulting to C++20")
    set(CMAKE_CXX_STANDARD 20)
endif ()

if (NOT DEFINED CMAKE_C_STANDARD)
    message(STATUS "No C version specified, defaulting to C17")
    set(CMAKE_C_STANDARD 17)
endif ()

# Detect bit size
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_compile_definitions(CPU_64_BIT)
else ()
    add_compile_definitions(CPU_32_BIT)
endif ()

# Detect platform
if (WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
    link_libraries(kernel32.lib user32.lib)
    message(STATUS "Detected Windows platform")
elseif (APPLE)
    add_compile_definitions(PLATFORM_MACOS PLATFORM_UNIX)
    message(STATUS "Detected MacOS platform")
elseif (UNIX)
    if (NOT APPLE)
        add_compile_definitions(PLATFORM_LINUX)
    endif ()
    add_compile_definitions(PLATFORM_UNIX)
    message(STATUS "Detected Unixoid platform")
else ()
    message(FATAL_ERROR "Unsupported platform '${CMAKE_SYSTEM_NAME}'")
endif ()

# Detect build type
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_DEBUG)
    set(BUILD_DEBUG ON)

    # Enable safe STL, disable inlining and optimizations
    if (PLATFORM_LINUX AND COMPILER_GCC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O0 -fno-inline -D_GLIBCXX_DEBUG_BACKTRACE")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0 -fno-inline -D_GLIBCXX_DEBUG_BACKTRACE")
    endif ()

    # Enable checked iterators
    if (PLATFORM_WINDOWS AND COMPILER_MSVC)
        add_compile_definitions(_ITERATOR_DEBUG_LEVEL=1)
    endif ()
else ()
endif ()

# Use mold
find_program(CMX_MOLD_EXECUTABLE "mold")
if(CMX_MOLD_EXECUTABLE) # mold doesn't work with Clang properly yet..
    message(STATUS "Detected mold linker, substituting")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-ld=mold")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=mold")

    if(NOT DEFINED CMX_NUM_LINK_THREADS) # So we can pass this in via CLI
        cmake_host_system_information(RESULT CMX_NUM_LINK_THREADS QUERY NUMBER_OF_LOGICAL_CORES)
    endif() # NOT DEFINED NUM_LINK_THREADS

    message(STATUS "Using ${CMX_NUM_LINK_THREADS} threads for linking")
    add_link_options("LINKER:--threads,--thread-count=${CMX_NUM_LINK_THREADS}")
endif() # MOLD_EXECUTABLE AND NOT COMPILER_CLANG

include(FetchContent)

# Fetch DX Compiler
FetchContent_Declare(
        dxc
        GIT_REPOSITORY https://github.com/microsoft/DirectXShaderCompiler.git
        GIT_TAG main
)
FetchContent_Populate(dxc)

# Fetch fmt
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_Populate(fmt)
add_compile_definitions(FMT_HEADER_ONLY)

# Fetch spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x)
FetchContent_Populate(spdlog)

include(${CMAKE_CURRENT_SOURCE_DIR}/runtime/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/editor/CMakeLists.txt)