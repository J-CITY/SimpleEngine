#include "gpuMemoryMetal.h"
#if defined(METAL_BACKEND) && IKIGAI_ENABLE_MEMORY_TRACKING
#import <objc/runtime.h>
#import <TargetConditionals.h>

// The record follows the native object's retain count, including command-buffer
// references. It never retains the resource, so no ownership cycle is created.
@interface IkigaiGpuMemoryRecord : NSObject {
@public
    IKIGAI::PROFILER::GpuAllocationToken token;
}
@end
@implementation IkigaiGpuMemoryRecord
@end

namespace IKIGAI::RENDER {
namespace {
char MemoryKey;
void Attach(id object, PROFILER::GpuAllocationInfo info) noexcept {
    if (!object) return;
    IkigaiGpuMemoryRecord* record = objc_getAssociatedObject(object, &MemoryKey);
    if (!record) {
        record = [[IkigaiGpuMemoryRecord alloc] init];
        if (!record) { PROFILER::GpuMemoryProfiler::instance().noteDroppedRecord(); return; }
        objc_setAssociatedObject(object, &MemoryKey, record, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    record->token.track(info);
}
}
void TrackGpuMemoryMetal(id<MTLResource> resource, PROFILER::GpuResourceKind kind, const char* name) noexcept {
    if (!resource) return;
    PROFILER::GpuAllocationInfo info;
    info.backend = PROFILER::GpuBackend::Metal;
    info.device = reinterpret_cast<std::uintptr_t>((__bridge const void*)resource.device);
    info.handle = reinterpret_cast<std::uint64_t>((__bridge const void*)resource);
    info.kind = kind; info.name = name;
    info.hostVisible = resource.storageMode == MTLStorageModeShared;
#if TARGET_OS_OSX
    info.hostVisible = info.hostVisible || resource.storageMode == MTLStorageModeManaged;
#endif
    info.deviceLocal = resource.storageMode == MTLStorageModePrivate;
    info.measurement = PROFILER::GpuMeasurement::Unavailable;
    if (@available(macOS 10.13, iOS 11.0, *)) {
        if ([resource respondsToSelector:@selector(allocatedSize)]) {
            info.bytes = resource.allocatedSize;
            info.measurement = PROFILER::GpuMeasurement::ReportedAllocation;
        }
    }
    Attach(resource, info);
}
void TrackGpuObjectMetal(id object, id<MTLDevice> device, PROFILER::GpuResourceKind kind, const char* name) noexcept {
    PROFILER::GpuAllocationInfo info;
    info.backend = PROFILER::GpuBackend::Metal;
    info.device = reinterpret_cast<std::uintptr_t>((__bridge const void*)device);
    info.handle = reinterpret_cast<std::uint64_t>((__bridge const void*)object);
    info.kind = kind; info.name = name; info.measurement = PROFILER::GpuMeasurement::Unavailable;
    Attach(object, info);
}
std::shared_ptr<PROFILER::GpuMemorySampler> CreateMemorySamplerMetal(id<MTLDevice> device) {
    return std::make_shared<PROFILER::GpuMemorySampler>([device] {
        PROFILER::GpuHeapSnapshot heap;
        heap.backend = PROFILER::GpuBackend::Metal;
        heap.device = reinterpret_cast<std::uintptr_t>((__bridge const void*)device);
        heap.capacityAvailable = heap.allocatorStatsAvailable = heap.usageAvailable = heap.budgetAvailable = false;
        if (@available(macOS 10.13, iOS 11.0, *)) {
            if ([device respondsToSelector:@selector(currentAllocatedSize)]) heap.deviceAllocatedBytes = device.currentAllocatedSize;
        }
        if (@available(macOS 10.12, iOS 16.0, *)) {
            if ([device respondsToSelector:@selector(recommendedMaxWorkingSetSize)]) heap.recommendedWorkingSetBytes = device.recommendedMaxWorkingSetSize;
        }
        if (@available(macOS 10.15, iOS 13.0, *)) {
            if ([device respondsToSelector:@selector(hasUnifiedMemory)]) heap.unifiedMemory = static_cast<bool>(device.hasUnifiedMemory);
        }
        return std::vector{heap};
    });
}
}
#endif
