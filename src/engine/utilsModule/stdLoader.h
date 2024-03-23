#pragma once

namespace IKIGAI::UTILS {
	void STBiSetFlipVerticallyOnLoad(bool b);
	float* STBiLoadf(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
	unsigned char* STBiLoad(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
	void STBiImageFree(float*);
	void STBiImageFree(unsigned char* data);
}
