#include "glManager.h"
#include <string>
import logger;

using namespace KUMA::GL_SYSTEM;

GlManager::GlManager(const DriverSettings& driverSettings) {
	initGlew();

	if (driverSettings.debugMode) {
		GLint flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(GLDebugMessageCallback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
}

void GlManager::GLDebugMessageCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char* message, const void* userParam) {
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
		return;
	}

	std::string output;

	output += "OpenGL Debug Message:\n";
	output += "Debug message (" + std::to_string(id) + "): " + message + "\n";

	switch (source) {
		case GL_DEBUG_SOURCE_API:				output += "Source: API";				break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		output += "Source: Window System";		break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:	output += "Source: Shader Compiler";	break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:		output += "Source: Third Party";		break;
		case GL_DEBUG_SOURCE_APPLICATION:		output += "Source: Application";		break;
		case GL_DEBUG_SOURCE_OTHER:				output += "Source: Other";				break;
	}

	output += "\n";

	switch (type) {
		case GL_DEBUG_TYPE_ERROR:               output += "Type: Error";				break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: output += "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  output += "Type: Undefined Behaviour";	break;
		case GL_DEBUG_TYPE_PORTABILITY:         output += "Type: Portability";			break;
		case GL_DEBUG_TYPE_PERFORMANCE:         output += "Type: Performance";			break;
		case GL_DEBUG_TYPE_MARKER:              output += "Type: Marker";				break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          output += "Type: Push Group";			break;
		case GL_DEBUG_TYPE_POP_GROUP:           output += "Type: Pop Group";			break;
		case GL_DEBUG_TYPE_OTHER:               output += "Type: Other";				break;
	}

	output += "\n";

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:			output += "Severity: High";				break;
		case GL_DEBUG_SEVERITY_MEDIUM:			output += "Severity: Medium";			break;
		case GL_DEBUG_SEVERITY_LOW:				output += "Severity: Low";				break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:	output += "Severity: Notification";		break;
	}

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:			LOG_ERROR(output);	break;
		case GL_DEBUG_SEVERITY_MEDIUM:			LOG_WARNING(output);	break;
		case GL_DEBUG_SEVERITY_LOW:				LOG_INFO(output);		break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:	LOG_INFO(output);			break;
	}
}

void GlManager::initGlew() {
	const GLenum error = glewInit();
	if (error != GLEW_OK) {
		std::string message = "GlManager::ERROR init GLEW: ";
		std::string glewError = reinterpret_cast<const char*>(glewGetErrorString(error));
		LOG_INFO(message + glewError);
	}
}
