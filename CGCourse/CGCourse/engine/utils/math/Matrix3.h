

#include <array>

struct Matrix3 {
	std::array<float, 9> data;
	//static const Matrix3 Eye;

	Matrix3() {
		for (auto& d : data) {
			d = 0.0f;
		}
	}
	Matrix3(float val) {
		for (auto& d : data) {
			d = val;
		}
	}

	Matrix3(float element1, float element2, float element3,
		float element4, float element5, float element6,
		float element7, float element8, float element9) {
		data[0] = element1;
		data[1] = element2;
		data[2] = element3;
		data[3] = element4;
		data[4] = element5;
		data[5] = element6;
		data[6] = element7;
		data[7] = element8;
		data[8] = element9;
	}

	Matrix3(std::array<float, 9>&& in) {
		data = in;
	}

	Matrix3(const Matrix3& in){
		*this = in;
	}

};
