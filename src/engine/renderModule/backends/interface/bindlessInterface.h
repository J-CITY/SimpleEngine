#pragma once
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>
#include "textureInterface.h"
#include "storageBufferInterface.h"

namespace IKIGAI::RENDER {
enum class ShaderBindingMode { Bound, Bindless };
enum class VertexFetchMode { Attributes, Pulling };
struct BindlessCapabilities {
    bool sampledTextures = false;
    bool storageBuffers = false;
    bool storageImages = false;
    bool nonUniformIndexing = false;
    bool runtimeArrays = false;
    uint32_t maxSampledTextures = 0;
    uint32_t maxStorageBuffers = 0;
};
struct BindlessResourceHandle {
    uint32_t binding = 0, index = 0, generation = 0;
};
struct BindlessResourceEntry {
    std::shared_ptr<TextureInterface> texture;
    std::shared_ptr<StorageBufferInterface> buffer;
    uint32_t generation = 1;
};
// A binding has one resource type and a fixed capacity. Slot zero is its default.
// Released slots are tombstones, never recycled into unrelated GPU resources.
// This also keeps indices in persistent material/mesh buffers safe.
struct BindlessResourceTable {
    uint32_t capacity = 0;
    std::vector<BindlessResourceEntry> entries;
    uint64_t revision = 0;
    BindlessResourceHandle add(uint32_t binding, BindlessResourceEntry value, uint32_t limit) {
        if (!value.texture && !value.buffer) throw std::invalid_argument("Null bindless resource");
        if (!capacity) capacity = limit;
        if (!limit || capacity != limit) throw std::invalid_argument("Bindless table capacity changed");
        if (!entries.empty()) {
            const auto& fallback=entries.front();
            if (bool(fallback.texture) != bool(value.texture) ||
                (value.texture && fallback.texture->getType()!=value.texture->getType()))
                throw std::invalid_argument("Bindless resource type differs from table default");
        }
        for (uint32_t i=0;i<entries.size();++i)
            if (entries[i].texture==value.texture && entries[i].buffer==value.buffer)
                return {binding,i,entries[i].generation};
        if (entries.size()>=capacity) throw std::runtime_error("Bindless table capacity exhausted");
        const auto index=static_cast<uint32_t>(entries.size());entries.push_back(std::move(value));++revision;
        return {binding,index,entries.back().generation};
    }
    const BindlessResourceEntry& get(uint32_t index) const {
        if (entries.empty()) throw std::runtime_error("Bindless table needs a default resource at slot zero");
        if (index>=entries.size() || (!entries[index].texture && !entries[index].buffer)) return entries.front();
        return entries[index];
    }
    void release(BindlessResourceHandle handle) {
        if (!handle.index) throw std::invalid_argument("Cannot release bindless default slot");
        if (handle.index>=entries.size() || entries[handle.index].generation!=handle.generation)
            throw std::invalid_argument("Stale bindless resource handle");
        auto& entry=entries[handle.index];entry.texture.reset();entry.buffer.reset();++entry.generation;++revision;
    }
};
}
