export module customarray;


export template<typename T, size_t N>
class Array {
public:
	Array() {
		for (auto i = 0; i < N; i++) {
			_values[i] = 0;
		}
	}

	Array(float* data) {
		for (auto i = 0; i < N; i++) {
			_values[i] = data[i];
		}
	}

	const size_t size() const {
		return N;
	}

	T& operator [](int idx) {
		return _values[idx];
	}

	const T& operator [](int idx) const {
		return _values[idx];
	}

	T& operator=(const T& other) {
		if (this == &other) {
			return *this;
		}
		if (N != other.size()) {
			
		}

		for (auto i = 0; i < N; i++) {
			_values[i] = other._values[i];
		}
		return *this;
	}
//private:
	T _values[N];
};
