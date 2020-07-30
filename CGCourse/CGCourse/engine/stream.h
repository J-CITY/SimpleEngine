#pragma once

#include <fstream>

#include "config.h"

namespace SE {
	class Stream {
	public:
		Stream(const std::string& fileName) : fileName(fileName) {
			fileStream = std::fstream(Config::ASSETS_PATH + fileName, std::ifstream::binary | std::fstream::in | std::fstream::out);
		}

		~Stream() {
		}

		size_t read(std::string& buf, size_t sz, size_t count) {
			fileStream.read((char*)&buf, sz * count);
			return fileStream.gcount();
		}

		size_t fileSize() {
			size_t cur = fileStream.tellg();
			fileStream.seekg(0, std::ios::end);

			size_t end = fileStream.tellg();
			fileStream.seekg(cur, std::ios::beg);

			return end;
		}

		std::string getFileName() const {
			return fileName;
		}

	private:
		std::string fileName;
		std::fstream fileStream;
	};
};
