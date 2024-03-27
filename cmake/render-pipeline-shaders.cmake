function(compile_rpsl TARGET_NAME RPSL_FILE_NAME OUTPUT_SOURCES)
    # Set platform dependant variables
    if (PLATFORM_WINDOWS OR PLATFORM_MACOS)
        set(RPS_COMPILER_BINARY_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/_deps/rps-src/tools/rps_hlslc/win-x64")
        set(RPS_DXCOMPILER_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/extern/dxcompiler/windows/${ARCH}/dxcompiler.dll")
        set(RPS_EXECUTABLE_POSTFIX ".exe")
    elseif (PLATFORM_LINUX)
        set(RpsOnLinux ON)
        set(RPS_COMPILER_BINARY_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/_deps/rps-src/tools/rps_hlslc/linux-x64/bin")
        set(RPS_DXCOMPILER_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/extern/dxcompiler/linux/libdxcompiler.so")
        set(RPS_EXECUTABLE_POSTFIX "")
    else ()
        message(FATAL_ERROR "Invalid platform '${CMAKE_SYSTEM_NAME}'")
    endif ()

    # Set platform independent variables
    get_filename_component(FILE_NAME ${RPSL_FILE_NAME} NAME_WE)
    set(RPS_HLSLC_FILE "${RPS_COMPILER_BINARY_FOLDER}/rps-hlslc${RPS_EXECUTABLE_POSTFIX}")
    set(RPS_LLVM_CBE_FILE "${RPS_COMPILER_BINARY_FOLDER}/llvm-cbe${RPS_EXECUTABLE_POSTFIX}")
    set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.tmp/generated-rpsl")
    set(OUTPUT_FILE "${OUTPUT_DIRECTORY}/${FILE_NAME}.rpsl.g.c")
    file(MAKE_DIRECTORY "${OUTPUT_DIRECTORY}")

    # Add command
    if (PLATFORM_MACOS)
        add_custom_command(
                OUTPUT "${OUTPUT_FILE}"
                COMMAND "/Applications/CrossOver.app/Contents/SharedSupport/Crossover/CrossOver-Hosted Application/wine"
                "--bottle" "RPS" "--cx-app" "${RPS_HLSLC_FILE}" "${RPSL_FILE_NAME}" -od "${OUTPUT_DIRECTORY}" -m ${FILE_NAME}
                WORKING_DIRECTORY "${OUTPUT_DIRECTORY}"
                DEPENDS "${RPSL_FILE_NAME}" "${RPS_HLSLC_FILE}" "${RPS_LLVM_CBE_FILE}" "${RPS_DXCOMPILER_LIBRARY_PATH}"
                VERBATIM
        )
    else ()
        add_custom_command(
                OUTPUT "${OUTPUT_FILE}"
                COMMAND "${RPS_HLSLC_FILE}" "${RPSL_FILE_NAME}" -od "${OUTPUT_DIRECTORY}" -m ${FILE_NAME}
                WORKING_DIRECTORY "${OUTPUT_DIRECTORY}"
                DEPENDS "${RPSL_FILE_NAME}" "${RPS_HLSLC_FILE}" "${RPS_LLVM_CBE_FILE}" "${RPS_DXCOMPILER_LIBRARY_PATH}"
                VERBATIM
        )
    endif ()
    set(${OUTPUT_SOURCES} "${OUTPUT_FILE}" PARENT_SCOPE)
endfunction()