#include "gpuMemoryDx12.h"
#if defined(DX12_BACKEND) && IKIGAI_ENABLE_MEMORY_TRACKING
#include <atomic>
#include <new>
#include <wrl/client.h>

namespace IKIGAI::RENDER {
namespace {
constexpr GUID MemoryKey{0xb446739b, 0x647f, 0x428d, {0xad, 0xba, 0x13, 0xe4, 0x6a, 0x81, 0xe0, 0x57}};
class Record final : public IUnknown {
public:
    PROFILER::GpuAllocationToken token;
    PROFILER::GpuAllocationInfo info;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) override {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (iid != __uuidof(IUnknown)) return E_NOINTERFACE;
        *result = static_cast<IUnknown*>(this); AddRef(); return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++mReferences; }
    ULONG STDMETHODCALLTYPE Release() override { const auto remaining = --mReferences; if (!remaining) delete this; return remaining; }
private:
    std::atomic<ULONG> mReferences{1};
};
Microsoft::WRL::ComPtr<Record> Find(ID3D12Object* object) noexcept {
    Microsoft::WRL::ComPtr<Record> result;
    IUnknown* data = nullptr; UINT bytes = sizeof(data);
    // GetPrivateData AddRefs interface data. The temporary releases that ref.
    if (object && SUCCEEDED(object->GetPrivateData(MemoryKey, &bytes, &data)) && data)
        result.Attach(static_cast<Record*>(data));
    return result;
}
}
void TrackGpuMemoryDx12(ID3D12Object* object, const PROFILER::GpuAllocationInfo& info) noexcept {
    if (!object) return;
    auto record = Find(object);
    if (!record) {
        record.Attach(new(std::nothrow) Record);
        if (!record) { PROFILER::GpuMemoryProfiler::instance().noteDroppedRecord(); return; }
        if (FAILED(object->SetPrivateDataInterface(MemoryKey, record.Get()))) {
            PROFILER::GpuMemoryProfiler::instance().noteDroppedRecord(); return;
        }
    }
    record->info = info; record->info.name = nullptr;
    record->token.track(info);
}
void SetGpuMemoryKindDx12(ID3D12Object* object, PROFILER::GpuResourceKind kind, const char* name) noexcept {
    if (auto record = Find(object)) {
        record->info.kind = kind;
        auto info = record->info; info.name = name;
        record->token.track(info);
    }
}
void MarkGpuMemoryPendingDx12(ID3D12Object* object) noexcept {
    if (auto record = Find(object)) record->token.markPending();
}
void TrackGpuObjectDx12(ID3D12Object* object, ID3D12Device* device, PROFILER::GpuResourceKind kind, const char* name) noexcept {
    PROFILER::GpuAllocationInfo info;
    info.backend = PROFILER::GpuBackend::DirectX12; info.device = reinterpret_cast<std::uintptr_t>(device);
    info.handle = reinterpret_cast<std::uint64_t>(object); info.kind = kind; info.name = name;
    info.measurement = PROFILER::GpuMeasurement::Unavailable;
    TrackGpuMemoryDx12(object, info);
}
}
#endif
