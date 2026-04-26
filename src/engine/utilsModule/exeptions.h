#pragma once
#include <stdexcept>

namespace IKIGAI::UTILS::EXEPTIONS
{
	class IndexOutOfRange final : public std::runtime_error {
	public:
		IndexOutOfRange() : std::runtime_error("Index is out of range") {}
	};
}
