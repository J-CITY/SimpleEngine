cmake_minimum_required(VERSION 3.20)
project(PhysicsBuildFlags LANGUAGES CXX)
get_filename_component(root "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
include("${root}/cmake/Physics2D.cmake")
include("${root}/cmake/Physics3D.cmake")
# Exercise the same glob/filter path used by the platform engine projects.
file(GLOB_RECURSE sources "${root}/src/engine/physicsModule/*.cpp"
    "${root}/src/engine/physics2dModule/*.cpp"
    "${root}/src/engine/coreModule/ecs/components/*physics*.cpp"
    "${root}/src/engine/coreModule/ecs/components/forceField2dComponent.cpp"
    "${root}/src/engine/coreModule/ecs/components/cloth2dComponent.cpp"
    "${root}/src/engine/coreModule/ecs/systems/physics*.cpp"
    "${root}/src/engine/renderModule/physics/*.cpp"
    "${root}/src/engine/renderModule/spatial/frustumBvh.h"
    "${root}/src/engine/generated/*physics*.cpp"
    "${root}/src/engine/generated/forceField2dComponent.generated.cpp"
    "${root}/src/engine/generated/cloth2dComponent.generated.cpp")
add_executable(engine "${root}/src/main.cpp" ${sources})
ikigai_configure_physics2d(engine)
ikigai_configure_physics3d(engine)
get_target_property(engine_sources engine SOURCES)
get_target_property(engine_links engine LINK_LIBRARIES)
get_target_property(engine_definitions engine COMPILE_DEFINITIONS)
foreach(dimension 2 3)
    if(USE_PHYSICS_${dimension}D)
        if(NOT TARGET ikigai_physics${dimension}d OR NOT "ikigai_physics${dimension}d" IN_LIST engine_links)
            message(FATAL_ERROR "Enabled ${dimension}D kernel was not linked")
        endif()
    else()
        if(TARGET ikigai_physics${dimension}d OR "ikigai_physics${dimension}d" IN_LIST engine_links)
            message(FATAL_ERROR "Disabled ${dimension}D kernel was built/linked")
        endif()
        foreach(source IN LISTS engine_sources)
            if(dimension EQUAL 3)
                if(source MATCHES "physicsModule/")
                    message(FATAL_ERROR "Disabled 3D simulation source retained: ${source}")
                endif()
                if(source MATCHES "(physics3dComponent|physicsComponent|physics3dSystem|deformableRenderer)(\\.generated)?\\.cpp$")
                    message(FATAL_ERROR "Disabled 3D integration retained: ${source}")
                endif()
            elseif(source MATCHES "physics2dModule/|((physics2dComponent|forceField2dComponent|cloth2dComponent|physics2dSystem)(\\.generated)?\\.cpp$)")
                message(FATAL_ERROR "Disabled 2D source retained: ${source}")
            endif()
        endforeach()
    endif()
    if(NOT "USE_PHYSICS_${dimension}D=$<BOOL:${USE_PHYSICS_${dimension}D}>" IN_LIST engine_definitions)
        message(FATAL_ERROR "Missing ${dimension}D compile flag")
    endif()
endforeach()
if(NOT "${root}/src/engine/renderModule/spatial/frustumBvh.h" IN_LIST engine_sources)
    message(FATAL_ERROR "Render BVH was removed with physics")
endif()
