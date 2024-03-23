cmake_minimum_required(VERSION 3.4.1)

set(APP_NAME "IkigaiEngine")

project(${APP_NAME})

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../3rd")
set(MAIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)

# if SDL2
set(LIB_SDL2 ${THIRD_PARTY_DIR}/SDL2_windows/lib/x64/SDL2.lib)
set(LIB_SDL2_MAIN ${THIRD_PARTY_DIR}/SDL2_windows/lib/x64/SDL2main.lib)

# if GLFW
#add_subdirectory(${THIRD_PARTY_DIR}/glfw ${THIRD_PARTY_DIR}/glfw/build)

# if OPENGL
set(LIB_GLEW ${THIRD_PARTY_DIR}/glew/lib/Release/x64/glew32s.lib)

add_subdirectory(${THIRD_PARTY_DIR}/assimp ./3rd/assimp/build)
add_subdirectory(${THIRD_PARTY_DIR}/freetype ./3rd/freetype/build)

include_directories(${THIRD_PARTY_DIR}/SDL2_windows/include)

# if OPENGL
include_directories(${THIRD_PARTY_DIR}/glew/include)

# if VULKAN
#include_directories(${THIRD_PARTY_DIR}/VulkanSDK/Include)


include_directories(${THIRD_PARTY_DIR}/glm)
include_directories(${THIRD_PARTY_DIR}/serdepp/include)
include_directories(${THIRD_PARTY_DIR}/magic_enum/include)
include_directories(${THIRD_PARTY_DIR}/nameof/include)
include_directories(${THIRD_PARTY_DIR}/json/single_include)
include_directories(${THIRD_PARTY_DIR}/assimp/include)
include_directories(${THIRD_PARTY_DIR}/soloud/include)
include_directories(${THIRD_PARTY_DIR}/fmt/include)

#if GLFW
include_directories(${THIRD_PARTY_DIR}/glfw/include)

# global engine include
include_directories(${MAIN_SOURCE_DIR}/engine)

# if USE_EDITOR or USE_CHEATS
file(GLOB_RECURSE IMGUI_FILES
     "../3rd/imgui/imgui/imconfig.h"
     "../3rd/imgui/imgui/imgui.cpp"
     "../3rd/imgui/imgui/imgui.h"
     "../3rd/imgui/imgui/imgui_draw.cpp"

     #if GLFW
     #"../3rd/imgui/imgui/imgui_impl_glfw.cpp"
     #"../3rd/imgui/imgui/imgui_impl_glfw.h"

     #if SDL2
     "../3rd/imgui/imgui/backends/imgui_impl_sdl2.h"
     "../3rd/imgui/imgui/backends/imgui_impl_sdl2.cpp"
     
     #OPENGL
     "../3rd/imgui/imgui/imgui_impl_opengl3.h"
     "../3rd/imgui/imgui/imgui_impl_opengl3.cpp"
     "../3rd/imgui/imgui/imgui_impl_opengl3_loader.h"

     #VULKAN
     #"../3rd/imgui/imgui/imgui_impl_vulkan.h"
     #"../3rd/imgui/imgui/imgui_impl_vulkan.cpp"

     # DX12
     #"../3rd/imgui/imgui/imgui_impl_dx12.h"
     #"../3rd/imgui/imgui/imgui_impl_dx12.cpp"
     #"../3rd/imgui/imgui/imgui_impl_win32.h"
     #"../3rd/imgui/imgui/imgui_impl_win32.cpp"

     "../3rd/imgui/imgui/imgui_internal.h"
     "../3rd/imgui/imgui/imgui_tables.cpp"
     "../3rd/imgui/imgui/imgui_widgets.cpp"
     "../3rd/imgui/imgui/imstb_rectpack.h"
     "../3rd/imgui/imgui/imstb_rectpack.h"
     "../3rd/imgui/imgui/imstb_textedit.h"
     "../3rd/imgui/imgui/imstb_truetype.h"
     "../3rd/imgui/imgui/misc/cpp/imgui_stdlib.h"
     "../3rd/imgui/imgui/misc/cpp/imgui_stdlib.cpp"

     "../3rd/imgui/IconFont/IconsFontAwesome5.h"
     "../3rd/imgui/imgui/GraphEditor.h"
     "../3rd/imgui/imgui/GraphEditor.cpp"
     "../3rd/imgui/imgui/ImCurveEdit.h"
     "../3rd/imgui/imgui/ImCurveEdit.cpp"
     "../3rd/imgui/imgui/ImGradient.h"
     "../3rd/imgui/imgui/ImGradient.cpp"
     "../3rd/imgui/imgui/ImZoomSlider.h"
)

file(GLOB_RECURSE SPINE_FILES
     "../3rd/spine/spine/*.h"
     "../3rd/spine/spine/*.cpp"
)

file(GLOB_RECURSE LUA_FILES
     "../3rd/lua/lapi.c"
     "../3rd/lua/lcode.c"
     "../3rd/lua/lctype.c"
     "../3rd/lua/ldebug.c"
     "../3rd/lua/ldo.c"
     "../3rd/lua/ldump.c"
     "../3rd/lua/lfunc.c"
     "../3rd/lua/lgc.c"
     "../3rd/lua/llex.c"
     "../3rd/lua/lmem.c"
     "../3rd/lua/lobject.c"
     "../3rd/lua/lopcodes.c"
     "../3rd/lua/lparser.c"
     "../3rd/lua/lstate.c"
     "../3rd/lua/lstring.c"
     "../3rd/lua/ltable.c"
     "../3rd/lua/ltm.c"
     "../3rd/lua/lundump.c"
     "../3rd/lua/lvm.c"
     "../3rd/lua/lzio.c"
     "../3rd/lua/lauxlib.c"
     "../3rd/lua/lbaselib.c"
     "../3rd/lua/lcorolib.c"
     "../3rd/lua/ldblib.c"
     "../3rd/lua/liolib.c"
     "../3rd/lua/lmathlib.c"
     "../3rd/lua/loadlib.c"
     "../3rd/lua/loslib.c"
     "../3rd/lua/lstrlib.c"
     "../3rd/lua/ltablib.c"
     "../3rd/lua/lutf8lib.c"
     "../3rd/lua/linit.c"
)

# if USE_EDITOR or USE_CHEATS
add_library(imgui STATIC ${IMGUI_FILES})

add_library(spine STATIC ${SPINE_FILES})
add_library(lua STATIC ${LUA_FILES})

include_directories("../3rd/sol/single/include")
target_include_directories(lua PUBLIC "../3rd/lua")
include_directories("../3rd/lua")
include_directories("../3rd/freetype/include")
include_directories("../3rd/spine")
target_include_directories(spine  PUBLIC "../3rd/spine")

# if USE_EDITOR or USE_CHEATS
target_include_directories(imgui PUBLIC "../3rd/imgui/imgui")
include_directories("../3rd/imgui/imgui")
include_directories("../3rd/imgui/IconFont")

file(GLOB_RECURSE CPP_HEADERS ${MAIN_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE CPP_HEADERS2 ${MAIN_SOURCE_DIR}/*.h)
file(GLOB_RECURSE CPP_SOURCES ${MAIN_SOURCE_DIR}/*.cpp)

if(CMAKE_BUILD_TYPE MATCHES "Release")
    set(EXECUTABLE_TYPE WIN32)
endif()

# for tree view in VS
source_group(TREE ${MAIN_SOURCE_DIR} FILES ${CPP_HEADERS} ${CPP_HEADERS2} ${CPP_SOURCES})

add_executable(
    ${APP_NAME} ${EXECUTABLE_TYPE}
    ${CPP_HEADERS}
    ${CPP_HEADERS2}
    ${CPP_SOURCES}
)

set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# DEFINES
add_compile_definitions(NOMINMAX)

# if SDL2
add_compile_definitions(USE_SDL)

# if GLFW
# add_compile_definitions(USE_GLFW)

# if EDITOR
add_compile_definitions(USE_EDITOR)

# if CHEATS
add_compile_definitions(USE_CHEATS)

# if (backend == opengl)
add_compile_definitions(OPENGL_BACKEND)
add_compile_definitions(OPENGL_SIMPLE_RENDER)
#add_compile_definitions(OPENGL_HARD_RENDER)

# if (backend == vulkan)
#add_compile_definitions(VULKAN_BACKEND)

# if (backend == directx12)
#add_compile_definitions(DX12_BACKEND)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
#add_compile_definitions(UNICODE)

target_compile_options(${APP_NAME} PRIVATE /bigobj)


target_link_libraries(
    ${APP_NAME}

    #if OPENGL
    opengl32
    ${LIB_GLEW}

    #if VULKAN
    #${THIRD_PARTY_DIR}/VulkanSDK/Lib/vulkan-1.lib

    # if SDL2
    ${LIB_SDL2}
    ${LIB_SDL2_MAIN}

    ${THIRD_PARTY_DIR}/glfw/build/src/Debug/glfw3ddll.lib

    ${THIRD_PARTY_DIR}/assimp/build/lib/Debug/assimp-vc143-mtd.lib
    ${THIRD_PARTY_DIR}/../windows/build/3rd/freetype/build/Debug/freetyped.lib
    imgui
    spine
    lua
    ${THIRD_PARTY_DIR}/soloud/lib/soloud_static_x64_d.lib
)
