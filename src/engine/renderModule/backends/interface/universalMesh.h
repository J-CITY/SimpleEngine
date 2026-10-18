#pragma once
#include "meshInterface.h"
#include "driverInterface.h"
#include "vertexBufferInterface.h"
#include "indexBufferInterface.h"
#include "../meshFactory.h"

namespace IKIGAI::RENDER {
class UniversalMesh : public MeshInterface {
protected:
    VertexLayout mLayout;
    std::vector<std::pair<uint32_t,std::shared_ptr<VertexBufferInterface>>> mStreams;
    std::shared_ptr<IndexBufferInterface> mIndexBuffer;
    DriverInterface* mDriver;
    std::vector<Vertex> mRayTracingVertices;
    std::vector<unsigned> mRayTracingIndices;
public:
    UniversalMesh(const MeshData& data, unsigned materialIndex)
        : mLayout(data.layout), mDriver(DriverInterface::Get()) {
        mLayout.validate();
        if (!mDriver || !data.vertexCount || data.vertexCount > UINT32_MAX)
            throw std::invalid_argument("Custom mesh requires an active driver and vertices");
        if (data.streams.size() != mLayout.streams.size())
            throw std::invalid_argument("Mesh stream count does not match layout");
        for (size_t i = 0; i < data.streams.size(); ++i) {
            const auto& stream = data.streams[i];
            const auto& layout = mLayout.stream(stream.binding);
            if (!stream.count || stream.count > SIZE_MAX / layout.stride ||
                stream.bytes.size() != stream.count * layout.stride ||
                (layout.rate == VertexInputRate::PerVertex && stream.count < data.vertexCount))
                throw std::invalid_argument("Custom mesh stream size/count does not match layout");
            for (size_t j = 0; j < i; ++j) if (data.streams[j].binding == stream.binding)
                throw std::invalid_argument("Duplicate mesh data stream binding");
        }
        if (data.indices.size() > UINT32_MAX) throw std::invalid_argument("Too many mesh indices");
        for (auto index : data.indices) if (index >= data.vertexCount)
            throw std::invalid_argument("Custom mesh index exceeds vertex count");
        mVertexCount = data.vertexCount; mIndicesCount = data.indices.size();
        mMaterialIndex = materialIndex; mBoundingSphere = data.bounds;
        if (!data.rayTracingVertices.empty()) {
            if (data.rayTracingVertices.size()!=data.vertexCount) throw std::invalid_argument("Canonical RT geometry must match mesh vertex count");
            mRayTracingVertices.assign(data.rayTracingVertices.begin(),data.rayTracingVertices.end());
            mRayTracingIndices.assign(data.indices.begin(),data.indices.end());
        }
    }
    void bind() const override {
        if (DriverInterface::Get() != mDriver) throw std::runtime_error("Mesh belongs to another driver");
        mDriver->setVertexStreams(mLayout,mStreams);
        mDriver->setIndexBuffer(mIndexBuffer);
    }
    void unbind() const override {}
    const VertexLayout* vertexLayout() const override { return &mLayout; }
    const std::vector<Vertex>& rayTracingVertices() const override { return mRayTracingVertices; }
    const std::vector<unsigned>& rayTracingIndices() const override { return mRayTracingIndices; }
    std::shared_ptr<IndexBufferInterface> rayTracingIndexBuffer() const override { return mIndexBuffer; }
    void updateVertexStream(uint32_t binding, std::span<const std::byte> bytes, size_t count) override {
        const auto& layout = mLayout.stream(binding);
        for (const auto& [slot, buffer] : mStreams) if (slot == binding) {
            if (count != buffer->getSize() || bytes.size() != buffer->getSizeByte())
                throw std::invalid_argument("Stream updates must preserve allocation and count");
            buffer->setData(bytes.data(),count,layout.stride);
            return;
        }
        throw std::invalid_argument("Unknown vertex stream binding");
    }
};
}
