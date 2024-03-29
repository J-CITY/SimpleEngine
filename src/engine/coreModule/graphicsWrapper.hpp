#pragma once

#if defined(__EMSCRIPTEN__)
#include <GLES3/gl3.h>
#define USING_GLES
#elif __APPLE__
#define GL_SILENCE_DEPRECATION
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE
#include <OpenGLES/ES2/gl.h>
#define USING_GLES
#else
#include <OpenGL/gl3.h>
#endif
#elif __ANDROID__
#include <GLES2/gl2.h>
#define USING_GLES
#elif WIN32

#ifdef OPENGL_BACKEND
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#ifdef VULKAN_BACKEND
#include <vulkan/vulkan.h>
#endif

#endif
