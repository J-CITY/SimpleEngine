#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>
// miniz-cpp uses std::uint64_t in its writer callback, but miniz expects
// unsigned long long. These are different types on some LP64 targets.
// Declare the exact overload before miniz-cpp assigns the callback; adapt the
// offset by value, without incompatible function-pointer casts or vendor edits.
namespace miniz_cpp::detail {
template <class Offset>
	requires(std::is_same_v<Offset, unsigned long long> && !std::is_same_v<Offset, std::uint64_t>)
inline std::size_t write_callback(void*, Offset, const void*, std::size_t);
}
#include "zip_file.hpp"
namespace miniz_cpp::detail {
template <class Offset>
	requires(std::is_same_v<Offset, unsigned long long> && !std::is_same_v<Offset, std::uint64_t>)
inline std::size_t write_callback(void* opaque, Offset offset, const void* data, std::size_t size) {
	return write_callback(opaque, static_cast<std::uint64_t>(offset), data, size);
}
}
