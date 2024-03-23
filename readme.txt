

Install SDL

- Win
Все либы в 3rd папку
Download https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip
Extract to ./3rd/SDL_windows folder

Download GLEW https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0-win32.zip
Extract to ./3rd/glew

# generate VS project
cmake -G "Visual Studio 17 2022" ./ -Bbuild

# скопировать dll в windows\out\
--------------------------------------------------------------
- Android

- copy SDL SDL_image to android\sdl\jni
- repace in  android\sdl\jni\SDL2_image\Android.mk: SUPPORT_WEBP ?= true to SUPPORT_WEBP ?= false
- copy assets folder to android\app\src\main\assets
- copy 3rd\SDL\android-project\app\src\main\java to android\sdl\src\main
- copy 3rd\SDL to android\sdl\jni

- В консоле Android Studio вызвать: ./gradlew :app:assembleDebug

- Build ASSIMP
Patch in assimp - ioapi.h #if defined(USE_FILE32API) -> #if defined(USE_FILE32API) || defined(ANDROID) 

Patch in Assimp - port/AndroidJNI/CMakeLists.txt
-INSTALL(TARGETS android_jniiosystem EXPORT "${TARGETS_EXPORT_NAME}")
+INSTALL(TARGETS android_jniiosystem EXPORT "${TARGETS_EXPORT_NAME}"
+       RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
+       LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
+               ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

glm patch
setup.hpp:143 #if GLM_PLATFORM == GLM_PLATFORM_ANDROI && !defined(GLM_LANG_STL11_FORCED)

freetype patch

CMakeList.txt:305
add_compile_definitions(HAVE_UNISTD_H)
add_compile_definitions(HAVE_FCNTL_H)

cmake -DCMAKE_TOOLCHAIN_FILE=C:\Users\DaniilGlushchenko\AppData\Local\Android\Sdk\ndk\26.0.10792818\build\cmake\android.toolchain.cmake -DANDROID_NDK=C:/Users/DaniilGlushchenko/AppData/Local/Android/Sdk/ndk/26.0.10792818 -DCMAKE_BUILD_TYPE=Release -DANDROID_ABI="arm64-v8a" -DASSIMP_ANDROID_JNIIOSYSTEM=ON DBUILD_SHARED_LIBS=1 . -B build -G Ninja
cmake --build .


---------------------------------------------------------------
- Emscripten
emcmake cmake . -B build -G Ninja
cmake --build build

Run local server
python -m http.server 8000