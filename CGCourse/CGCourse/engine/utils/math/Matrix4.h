

#include <array>

struct Matrix4 {
	std::array<float, 16> data;
	//static const Matrix3 Eye;

	Matrix4() {
		for (auto& d : data) {
			d = 0.0f;
		}
	}
	Matrix4(float val) {
		for (auto& d : data) {
			d = val;
		}
	}

	Matrix4(float element1, float element2, float element3, float element4, 
		float element5, float element6, float element7, float element8, 
		float element9, float element10, float element11, float element12,
		float element13, float element14, float element15, float element16) {
		data[0] = element1;
		data[1] = element2;
		data[2] = element3;
		data[3] = element4;
		data[4] = element5;
		data[5] = element6;
		data[6] = element7;
		data[7] = element8;
		data[8] = element9;
		data[8] = element10;
		data[8] = element11;
		data[8] = element12;
		data[8] = element13;
		data[8] = element14;
		data[8] = element15;
		data[8] = element16;
	}

	Matrix4(std::array<float, 16>&& in) {
		data = in;
	}

	Matrix4(const Matrix4& in){
		*this = in;
	}

};
