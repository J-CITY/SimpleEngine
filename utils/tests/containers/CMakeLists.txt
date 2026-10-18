cmake_minimum_required(VERSION 3.20)
project(IkigaiContainerTests LANGUAGES CXX)
if(MSVC)
    add_compile_options(/EHsc)
endif()
enable_testing()
find_package(Threads REQUIRED)
get_filename_component(IKIGAI_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
foreach(tracking 0 1)
    add_executable(ikigai_containers_${tracking} containers.cpp smallVector.cpp
        "${IKIGAI_ROOT}/src/engine/utilsModule/weakPtr.cpp"
        "${IKIGAI_ROOT}/src/engine/utilsModule/assertion.cpp"
        "${IKIGAI_ROOT}/src/engine/profilerModule/gpuMemoryProfiler.cpp"
        "${IKIGAI_ROOT}/src/engine/profilerModule/processMemory.cpp")
    target_compile_features(ikigai_containers_${tracking} PRIVATE cxx_std_20)
    target_compile_definitions(ikigai_containers_${tracking} PRIVATE NOMINMAX IKIGAI_ENABLE_MEMORY_TRACKING=${tracking})
    target_include_directories(ikigai_containers_${tracking} PRIVATE "${IKIGAI_ROOT}/src/engine"
        "${IKIGAI_ROOT}/3rd/json/single_include" "${IKIGAI_ROOT}/3rd/serdepp/include"
        "${IKIGAI_ROOT}/3rd/nameof/include" "${IKIGAI_ROOT}/3rd/magic_enum/include"
        "${IKIGAI_ROOT}/3rd/magic_enum/include/magic_enum" "${IKIGAI_ROOT}/3rd/fmt/include")
    target_link_libraries(ikigai_containers_${tracking} PRIVATE Threads::Threads)
    add_test(NAME containers_tracking_${tracking} COMMAND ikigai_containers_${tracking})
    set_tests_properties(containers_tracking_${tracking} PROPERTIES TIMEOUT 30)
endforeach()
add_executable(ikigai_containers_scene_lifetime "${IKIGAI_ROOT}/utils/tests/scene_lifetime.cpp"
    "${IKIGAI_ROOT}/src/engine/utilsModule/weakPtr.cpp")
target_compile_features(ikigai_containers_scene_lifetime PRIVATE cxx_std_20)
target_compile_definitions(ikigai_containers_scene_lifetime PRIVATE NOMINMAX IKIGAI_ENABLE_MEMORY_TRACKING=0)
target_include_directories(ikigai_containers_scene_lifetime PRIVATE "${IKIGAI_ROOT}/src/engine"
    "${IKIGAI_ROOT}/3rd/fmt/include" "${IKIGAI_ROOT}/3rd/nameof/include")
add_test(NAME containers_scene_lifetime COMMAND ikigai_containers_scene_lifetime)
set_tests_properties(containers_scene_lifetime PROPERTIES TIMEOUT 30)
option(IKIGAI_CONTAINER_BENCHMARKS "Build standalone container microbenchmarks" ON)
if(IKIGAI_CONTAINER_BENCHMARKS)
    add_executable(ikigai_container_benchmarks benchmarks.cpp)
    target_compile_features(ikigai_container_benchmarks PRIVATE cxx_std_20)
    target_compile_definitions(ikigai_container_benchmarks PRIVATE NOMINMAX IKIGAI_ENABLE_MEMORY_TRACKING=0)
    target_include_directories(ikigai_container_benchmarks PRIVATE "${IKIGAI_ROOT}/src/engine")
endif()
