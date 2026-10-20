include_guard(GLOBAL)

# Keep the reviewed fixes reproducible until they are published upstream and the
# submodule can be advanced. Never reset or overwrite unrelated local changes.
function(ikigai_prepare_argparse source)
    find_package(Git REQUIRED)
    set(patch "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../patches/argparse.patch")
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${source}" apply --reverse --check "${patch}"
        RESULT_VARIABLE already_applied OUTPUT_QUIET ERROR_QUIET)
    if(already_applied EQUAL 0)
        return()
    endif()
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${source}" apply --check "${patch}"
        RESULT_VARIABLE can_apply OUTPUT_QUIET ERROR_VARIABLE reason)
    if(NOT can_apply EQUAL 0)
        message(FATAL_ERROR "ArgParse fixes conflict with local changes: ${reason}")
    endif()
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${source}" apply "${patch}"
        RESULT_VARIABLE applied ERROR_VARIABLE reason)
    if(NOT applied EQUAL 0)
        message(FATAL_ERROR "Cannot apply reviewed ArgParse fixes: ${reason}")
    endif()
endfunction()
