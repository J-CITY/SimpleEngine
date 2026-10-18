#pragma once
#include <cstdint>
#include <optional>

namespace IKIGAI::PROFILER {
struct ProcessMemorySnapshot {
	std::optional<std::uint64_t> residentBytes, peakResidentBytes;
	std::optional<std::uint64_t> privateCommittedBytes, physicalFootprintBytes;
	std::optional<std::uint64_t> totalPhysicalBytes, availablePhysicalBytes;
};
// OS counters, sampled only when requested; independent of engine allocators.
ProcessMemorySnapshot CaptureProcessMemory() noexcept;
}
