#include "stdLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../3rd/stb/stb_image.h"

using namespace IKIGAI::UTILS;

void IKIGAI::UTILS::STBiSetFlipVerticallyOnLoad(bool b) {
	stbi_set_flip_vertically_on_load(b);
}

float* IKIGAI::UTILS::STBiLoadf(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels) {
	return stbi_loadf(filename, x, y, channels_in_file, desired_channels);
}

unsigned char* IKIGAI::UTILS::STBiLoad(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels) {
	return stbi_load(filename, x, y, channels_in_file, desired_channels);
}

void IKIGAI::UTILS::STBiImageFree(float* data) {
	stbi_image_free(data);
}

void IKIGAI::UTILS::STBiImageFree(unsigned char* data) {
	stbi_image_free(data);
}

