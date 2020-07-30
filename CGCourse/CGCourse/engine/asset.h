#pragma once
#include <memory>
#include <string>

#include "stream.h"

namespace SE {
	class Asset {
	public:
		Asset(const std::string& fileName) {
			stream = std::make_unique<Stream>(fileName);
			fileSize = stream->fileSize();
			buffer.resize(fileSize + 1);
			buffer[fileSize] = '\0';
		}

		~Asset() {}

		const std::string& read() {
			stream->read(buffer, sizeof(char), fileSize);
			return buffer;
		}

		const std::unique_ptr<Stream>& getIOStream() const {
			return stream;
		}

	private:
		std::string buffer;
		std::unique_ptr<Stream> stream;
		size_t fileSize;
	};
};
