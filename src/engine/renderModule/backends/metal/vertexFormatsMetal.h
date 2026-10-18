#pragma once
#ifdef METAL_BACKEND
#include <Metal/Metal.h>
#include "../interface/vertexLayout.h"
namespace IKIGAI::RENDER {
inline MTLVertexFormat NativeVertexFormat(VertexFormat format) {
    static constexpr MTLVertexFormat formats[] = {MTLVertexFormatFloat,MTLVertexFormatFloat2,MTLVertexFormatFloat3,MTLVertexFormatFloat4,MTLVertexFormatInt,MTLVertexFormatInt2,MTLVertexFormatInt3,MTLVertexFormatInt4,MTLVertexFormatUInt,MTLVertexFormatUInt2,MTLVertexFormatUInt3,MTLVertexFormatUInt4,MTLVertexFormatUChar4Normalized,MTLVertexFormatChar4Normalized};
    GetVertexFormatInfo(format);
    return formats[static_cast<unsigned>(format)];
}
}
#endif

