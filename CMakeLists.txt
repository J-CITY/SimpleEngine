# cmake -G "Visual Studio 17 2022" ./ -Bbuild

cmake_minimum_required(VERSION 3.8)

set(APP_NAME "IkigaiEngine")

set (CMAKE_CXX_STANDARD 20)

# 3rd
add_subdirectory(./3rd/assimp ./3rd/assimp/build)
add_custom_target(rttr cmake -G "Visual Studio 17 2022" ./3rd/rttr #-Bbuild
    WORKING_DIRECTORY ./3rd/rttr
)
add_subdirectory(./3rd/glfw ./3rd/glfw/build)
#add_subdirectory(./3rd/lua ./3rd/lua/build)
add_subdirectory(./3rd/freetype ./3rd/freetype/build)
#add_custom_target(freetype cmake -G "Visual Studio 17 2022" ./3rd/freetype #-Bbuild
#    WORKING_DIRECTORY ./3rd/freetype
#)


# Set project name
project(${APP_NAME})

# Project libs

# audioModule
#file(GLOB_RECURSE AUDIO_MODULE_H
#     "src/engine/audioModule/*.h"
#)
#file(GLOB_RECURSE AUDIO_MODULE_SRC
#     "src/engine/audioModule/*.cpp"
#)
#add_library(audioModule STATIC ${AUDIO_MODULE_H} ${AUDIO_MODULE_SRC})

# engine
file(GLOB_RECURSE ENGINE_MODULE_FILES
     "src/engine/*.h"
     "src/engine/*.ixx"
     "src/engine/*.cpp"
)
file(GLOB_RECURSE IMGUI_FILES
     "3rd/imgui/imgui/imconfig.h"
     "3rd/imgui/imgui/imgui.cpp"
     "3rd/imgui/imgui/imgui.h"
     "3rd/imgui/imgui/imgui_draw.cpp"
     "3rd/imgui/imgui/imgui_impl_glfw.cpp"
     "3rd/imgui/imgui/imgui_impl_glfw.h"

     #OPENGL
     "3rd/imgui/imgui/imgui_impl_opengl3.h"
     "3rd/imgui/imgui/imgui_impl_opengl3.cpp"
     "3rd/imgui/imgui/imgui_impl_opengl3_loader.h"
     #VULKAN
     # "3rd/imgui/imgui/imgui_impl_vulkan.h"
     # "3rd/imgui/imgui/imgui_impl_vulkan.cpp"
     # DX12
     #"3rd/imgui/imgui/imgui_impl_dx12.h"
     #"3rd/imgui/imgui/imgui_impl_dx12.cpp"
     #"3rd/imgui/imgui/imgui_impl_win32.h"
     #"3rd/imgui/imgui/imgui_impl_win32.cpp"

     "3rd/imgui/imgui/imgui_internal.h"
     "3rd/imgui/imgui/imgui_tables.cpp"
     "3rd/imgui/imgui/imgui_widgets.cpp"
     "3rd/imgui/imgui/imstb_rectpack.h"
     "3rd/imgui/imgui/imstb_rectpack.h"
     "3rd/imgui/imgui/imstb_textedit.h"
     "3rd/imgui/imgui/imstb_truetype.h"
     "3rd/imgui/imgui/misc/cpp/imgui_stdlib.h"
     "3rd/imgui/imgui/misc/cpp/imgui_stdlib.cpp"

     "3rd/imgui/IconFont/IconsFontAwesome5.h"
     "3rd/imgui/imgui/GraphEditor.h"
     "3rd/imgui/imgui/GraphEditor.cpp"
     "3rd/imgui/imgui/ImCurveEdit.h"
     "3rd/imgui/imgui/ImCurveEdit.cpp"
     "3rd/imgui/imgui/ImGradient.h"
     "3rd/imgui/imgui/ImGradient.cpp"
     # "src/3rd/imgui/imgui/ImGuizmo.h"
     # "src/3rd/imgui/imgui/ImGuizmo.cpp"
     # "src/3rd/imgui/imgui/ImSequencer.h"
     # "src/3rd/imgui/imgui/ImSequencer.cpp"
     "3rd/imgui/imgui/ImZoomSlider.h"
)

file(GLOB_RECURSE SPINE_FILES
     "3rd/spine/spine/*.h"
     "3rd/spine/spine/*.cpp"
)

add_library(engine STATIC ${ENGINE_MODULE_FILES})
add_library(imgui STATIC ${IMGUI_FILES})
add_library(spine STATIC ${SPINE_FILES})



file(GLOB_RECURSE LUA_FILES
     "3rd/lua/lapi.c"
     "3rd/lua/lcode.c"
     "3rd/lua/lctype.c"
     "3rd/lua/ldebug.c"
     "3rd/lua/ldo.c"
     "3rd/lua/ldump.c"
     "3rd/lua/lfunc.c"
     "3rd/lua/lgc.c"
     "3rd/lua/llex.c"
     "3rd/lua/lmem.c"
     "3rd/lua/lobject.c"
     "3rd/lua/lopcodes.c"
     "3rd/lua/lparser.c"
     "3rd/lua/lstate.c"
     "3rd/lua/lstring.c"
     "3rd/lua/ltable.c"
     "3rd/lua/ltm.c"
     "3rd/lua/lundump.c"
     "3rd/lua/lvm.c"
     "3rd/lua/lzio.c"
     "3rd/lua/lauxlib.c"
     "3rd/lua/lbaselib.c"
     "3rd/lua/lcorolib.c"
     "3rd/lua/ldblib.c"
     "3rd/lua/liolib.c"
     "3rd/lua/lmathlib.c"
     "3rd/lua/loadlib.c"
     "3rd/lua/loslib.c"
     "3rd/lua/lstrlib.c"
     "3rd/lua/ltablib.c"
     "3rd/lua/lutf8lib.c"
     "3rd/lua/linit.c"
)

add_library(lua STATIC ${LUA_FILES})


# executable project
add_executable(${APP_NAME} src/main.cpp)

# Project libs include
#target_include_directories(${APP_NAME} PUBLIC ${AUDIO_MODULE_H})
target_include_directories(${APP_NAME} PUBLIC "src/engine")
target_include_directories(engine PUBLIC "src/engine")

# Link project libs
#target_link_libraries(${APP_NAME} PUBLIC audioModule)
target_link_libraries(${APP_NAME} PUBLIC engine)


# assimp
target_link_libraries(${APP_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/build/3rd/assimp/build/lib/Debug/assimp-vc143-mtd.lib)
target_include_directories(engine PUBLIC "./3rd/assimp/include")

# soloud
target_link_libraries(${APP_NAME}  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3rd/soloud/lib/soloud_static_x64_d.lib)
target_include_directories(engine  PUBLIC "./3rd/soloud/include")

# rttr
target_link_libraries(${APP_NAME}  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3rd/rttr/lib/Debug/rttr_core_d.lib)
target_include_directories(engine  PUBLIC "./3rd/rttr/src")

# glfw
target_link_libraries(${APP_NAME}  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/build/3rd/glfw/build/src/Debug/glfw3ddll.lib)
target_include_directories(engine  PUBLIC "./3rd/glfw/include")

# glew
target_link_libraries(${APP_NAME}  PUBLIC "C:/libs/glew/lib/Release/x64/glew32.lib")
target_include_directories(engine  PUBLIC "C:/libs/glew/include")

# vulkan
target_link_libraries(${APP_NAME}  PUBLIC "C:/libs/VulkanSDK/Lib/vulkan-1.lib")
target_include_directories(engine  PUBLIC "C:/libs/VulkanSDK/Include")


#dx12


# sol
#target_link_libraries(${APP_NAME}  PUBLIC ./3rd/sol/single)
target_include_directories(engine  PUBLIC "./3rd/sol/single/include")

# lua
target_link_libraries(${APP_NAME}  PUBLIC lua)
#target_include_directories(engine  PUBLIC "./3rd/lua")
target_include_directories(lua PUBLIC "3rd/lua")
target_include_directories(engine PUBLIC "3rd/lua")

# freetype
target_link_libraries(${APP_NAME}  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/build/3rd/freetype/build/Debug/freetyped.lib)
target_include_directories(engine  PUBLIC "./3rd/freetype/include")

# jsom
target_include_directories(engine  PUBLIC "./3rd/json/single_include")

# magci enum
target_include_directories(engine  PUBLIC "./3rd/magic_enum/include")

# nameof
target_include_directories(engine  PUBLIC "./3rd/nameof/include")

# stb
target_include_directories(engine  PUBLIC "./3rd/stb")

# result
target_include_directories(engine  PUBLIC "./3rd/result")
# logger
target_include_directories(engine  PUBLIC "./3rd/logger")

# imgui
# target_include_directories(engine PUBLIC "./3rd/imgui")
target_link_libraries(${APP_NAME} PUBLIC imgui)
target_include_directories(engine PUBLIC "./3rd/imgui/imgui")
target_include_directories(imgui PUBLIC "./3rd/imgui/imgui")
target_include_directories(imgui  PUBLIC "./3rd/glfw/include")

# spine
target_link_libraries(${APP_NAME}  PUBLIC spine)
target_include_directories(engine  PUBLIC "./3rd/spine")
target_include_directories(spine  PUBLIC "./3rd/spine")

# VULKAN
# target_include_directories(imgui  PUBLIC "C:/libs/VulkanSDK/Include")

# glm
target_include_directories(engine  PUBLIC "./3rd/glm")

# if opengl
find_package(OpenGL REQUIRED)
target_link_libraries(${APP_NAME} PUBLIC opengl32)

# set startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${APP_NAME})

# add tree structure
FILE(GLOB_RECURSE headers "*.h")
FILE(GLOB_RECURSE sources "*.cpp")
FILE(GLOB_RECURSE modules "*.ixx")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources} ${headers} ${modules})

#OGL
add_compile_definitions(OPENGL_BACKEND)

#VK
# add_compile_definitions(VULKAN_BACKEND)

#DX12
# add_compile_definitions(DX12_BACKEND)
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
# add_compile_definitions(UNICODE)

# if MSVC
add_compile_definitions(NOMINMAX)

# if MSVC
#add_compile_options(/bigobj)
# if gcc
# add_compile_options(-Wa,-mbig-obj)
target_compile_options(engine PRIVATE /bigobj)

#set(CMAKE_EXE_LINKER_FLAGS "bigobj")
#set( CMAKE_C_FLAGS "bigobj")
#add_compile_definitions(VULKAN_BACKEND)
#add_compile_definitions(DX12_BACKEND)

