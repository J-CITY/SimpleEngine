#include "meshInterface.h"
#include "driverInterface.h"
#include "vertexBufferInterface.h"

using namespace IKIGAI::RENDER;

const BoundingSphere& MeshInterface::getBoundingSphere() const {
	return mBoundingSphere;
}

void MeshInterface::setOffset(size_t offset) {
	mOffset = offset;
}

void DriverInterface::validateVertexInputs(const ShaderInterface& shader) const {
    if (!mVertexLayout) return;
    for (const auto& input : shader.getReflection().mInputParams) {
        if (!input.mComponents) continue;
        const auto it = std::find_if(mVertexLayout->attributes.begin(), mVertexLayout->attributes.end(),
            [&](const auto& a) { return a.location == input.mLocation; });
        if (it == mVertexLayout->attributes.end())
            throw std::runtime_error("Missing vertex input " + input.mName + " in " + shader.mPath);
        const auto format = GetVertexFormatInfo(it->format);
        if (input.mBitWidth!=32 || format.components != input.mComponents || format.shaderType != input.mScalarType)
            throw std::runtime_error("Vertex input type mismatch for " + input.mName + " in " + shader.mPath);
    }
}

void DriverInterface::validateVertexDraw(const DrawArguments& args) const {
    if (!args.indexed && args.baseVertex) throw std::invalid_argument("Base vertex applies only to indexed draws");
    if (!mVertexLayout || !args.count || !args.instanceCount) return;
    for (const auto& layout : mVertexLayout->streams) {
        auto it = std::find_if(mVertexStreams.begin(),mVertexStreams.end(),
            [&](const auto& s){ return s.first == layout.binding; });
        if (it == mVertexStreams.end() || !it->second) throw std::runtime_error("Missing vertex buffer stream");
        if (it->second->getStride() != layout.stride) throw std::runtime_error("Vertex buffer stride mismatch");
        const uint64_t end = layout.rate == VertexInputRate::PerInstance
            ? uint64_t(args.firstInstance) + args.instanceCount
            : args.indexed ? 0 : uint64_t(args.first) + args.count;
        if (end > it->second->getSize()) throw std::runtime_error("Draw exceeds vertex/instance stream");
    }
}
