# Main-thread C++20 coroutines; independent of window, rendering and worker pools.
set(IKIGAI_COROUTINE_DIR "${CMAKE_CURRENT_LIST_DIR}/../src/engine/coroutineModule")
add_library(ikigai_coroutines STATIC
    "${IKIGAI_COROUTINE_DIR}/scheduler.cpp"
    "${IKIGAI_COROUTINE_DIR}/coroutineScope.cpp")
target_compile_features(ikigai_coroutines PUBLIC cxx_std_20)
target_include_directories(ikigai_coroutines PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../src/engine")
set_target_properties(ikigai_coroutines PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(ikigai_coroutines PRIVATE /W4 /utf-8)
    target_compile_options(ikigai_coroutines PUBLIC /EHsc)
else()
    target_compile_options(ikigai_coroutines PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(EMSCRIPTEN OR ANDROID)
    target_compile_options(ikigai_coroutines PUBLIC -fexceptions)
endif()

# Engine platform projects glob src/: compile this module only in its library.
foreach(coroutine_source_list PROJECT_ABSOLUTE_SOURCES PROJECT_SOURCES)
    list(FILTER ${coroutine_source_list} EXCLUDE REGEX "coroutineModule/.*\\.cpp$")
endforeach()

option(IKIGAI_COROUTINE_TESTS "Build independent main-thread coroutine tests" OFF)
if(IKIGAI_COROUTINE_TESTS)
    include("${CMAKE_CURRENT_LIST_DIR}/tests/Coroutines.cmake")
endif()
