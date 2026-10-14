include_guard(GLOBAL)

# Keep the SDK checkout pristine. Only DX12 and the validation CLI need the
# engine's HLSL ray-tracing extension, built from an archived SDK revision.
function(ikigai_prepare_spirv_cross sdk_dir output_variable)
    find_package(Git REQUIRED)
    set(patch "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../patches/spirv-cross-ray-tracing.patch")
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${sdk_dir}" rev-parse HEAD
        OUTPUT_VARIABLE revision OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE error RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Cannot read SPIRV-Cross revision; initialize the repository submodules. ${error}")
    endif()
    file(SHA256 "${patch}" patch_hash)
    string(SHA256 fingerprint "${revision}:${patch_hash}")
    set(source "${CMAKE_BINARY_DIR}/_deps/ikigai-spirv-cross-${fingerprint}")
    set(marker "${source}/.ikigai-patch-ready")
    # CMake must rerun when the extension or the selected SDK commit changes.
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${patch}")
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${sdk_dir}" rev-parse --absolute-git-dir
        OUTPUT_VARIABLE git_dir OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE error RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Cannot locate SPIRV-Cross Git metadata. ${error}")
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${git_dir}/HEAD")

    if(NOT EXISTS "${marker}")
        message(STATUS "Preparing SPIRV-Cross HLSL RT extension in the build directory")
        file(MAKE_DIRECTORY "${source}")
        set(archive "${source}/.ikigai-source.tar")
        execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${sdk_dir}" archive
            --format=tar "--output=${archive}" "${revision}"
            ERROR_VARIABLE error RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Cannot archive SPIRV-Cross ${revision}. ${error}")
        endif()
        execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xf "${archive}"
            WORKING_DIRECTORY "${source}" ERROR_VARIABLE error RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Cannot extract SPIRV-Cross source. ${error}")
        endif()
        file(REMOVE "${archive}")
        # Stop Git discovery at _deps so apply cannot use the engine's index.
        get_filename_component(ceiling "${source}" DIRECTORY)
        set(apply_command "${CMAKE_COMMAND}" -E env --unset=GIT_DIR
            --unset=GIT_WORK_TREE --unset=GIT_INDEX_FILE
            "GIT_CEILING_DIRECTORIES=${ceiling}" "${GIT_EXECUTABLE}" apply)
        execute_process(COMMAND ${apply_command} --check "${patch}"
            WORKING_DIRECTORY "${source}" ERROR_VARIABLE error RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "SPIRV-Cross ${revision} is incompatible with the HLSL RT patch; review patches/spirv-cross-ray-tracing.patch. ${error}")
        endif()
        execute_process(COMMAND ${apply_command} "${patch}"
            WORKING_DIRECTORY "${source}" ERROR_VARIABLE error RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Cannot apply the SPIRV-Cross HLSL RT extension. ${error}")
        endif()
        file(WRITE "${marker}" "${revision}\n${patch_hash}\n")
    endif()
    set(${output_variable} "${source}" PARENT_SCOPE)
endfunction()
