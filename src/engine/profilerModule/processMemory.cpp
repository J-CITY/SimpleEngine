#include "processMemory.h"
#ifndef IKIGAI_ENABLE_MEMORY_TRACKING
#define IKIGAI_ENABLE_MEMORY_TRACKING 0
#endif
#if IKIGAI_ENABLE_MEMORY_TRACKING
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifndef PSAPI_VERSION
#define PSAPI_VERSION 2
#endif
#include <psapi.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <fstream>
#include <sstream>
#include <string>
#endif
#endif

namespace IKIGAI::PROFILER {
ProcessMemorySnapshot CaptureProcessMemory() noexcept {
	ProcessMemorySnapshot result;
#if IKIGAI_ENABLE_MEMORY_TRACKING
#if defined(_WIN32)
	PROCESS_MEMORY_COUNTERS_EX process { };
	process.cb = sizeof(process);
	if (K32GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&process), sizeof(process))) {
		result.residentBytes = process.WorkingSetSize;
		result.peakResidentBytes = process.PeakWorkingSetSize;
		result.privateCommittedBytes = process.PrivateUsage;
	}
	MEMORYSTATUSEX system { };
	system.dwLength = sizeof(system);
	if (GlobalMemoryStatusEx(&system)) {
		result.totalPhysicalBytes = system.ullTotalPhys;
		result.availablePhysicalBytes = system.ullAvailPhys;
	}
#elif defined(__APPLE__)
	mach_task_basic_info_data_t basic { };
	mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
	if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&basic), &count) == KERN_SUCCESS) {
		result.residentBytes = basic.resident_size;
		result.peakResidentBytes = basic.resident_size_max;
	}
	task_vm_info_data_t vm { };
	count = TASK_VM_INFO_COUNT;
	if (task_info(mach_task_self(), TASK_VM_INFO, reinterpret_cast<task_info_t>(&vm), &count) == KERN_SUCCESS && count >= TASK_VM_INFO_REV1_COUNT)
		result.physicalFootprintBytes = vm.phys_footprint;
	std::uint64_t total = 0;
	size_t size = sizeof(total);
	if (sysctlbyname("hw.memsize", &total, &size, nullptr, 0) == 0)
		result.totalPhysicalBytes = total;
#elif defined(__linux__)
	try {
		auto read = [](const char* path, auto&& visit) {
			std::ifstream stream(path);
			std::string line;
			while (std::getline(stream, line)) {
				std::istringstream input(line);
				std::string key, unit;
				std::uint64_t value;
				if (input >> key >> value >> unit && unit == "kB")
					visit(key, value * 1024);
			}
		};
		read("/proc/self/status", [&](const auto& key, auto bytes) {
			if (key == "VmRSS:")
				result.residentBytes = bytes;
			if (key == "VmHWM:")
				result.peakResidentBytes = bytes;
		});
		read("/proc/meminfo", [&](const auto& key, auto bytes) {
			if (key == "MemTotal:")
				result.totalPhysicalBytes = bytes;
			if (key == "MemAvailable:")
				result.availablePhysicalBytes = bytes;
		});
	} catch (...) { } // Unavailable counters remain absent, never measured zero.
#endif
#endif
	return result;
}
}
