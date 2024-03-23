#pragma once

namespace IKIGAI::RENDER {
	enum class PrimitiveMode {
		POINTS = 0,
		LINES,
		LINE_LOO,
		LINE_STRIP,
		TRIANGLES,
		TRIANGLE_STRIP,
		TRIANGLE_FAN,
#ifndef USING_GLES
		LINES_ADJACENCY,
		LINE_STRIP_ADJACENCY,
		TRIANGLES_ADJACENCY,
		TRIANGLE_STRIP_ADJACENCY,
		PATCHES,
#endif
	};
	enum class RenderingCapability {
		BLEND = 0,
		CULL_FACE,
		DEPTH_TEST,
		DITHER,
		POLYGON_OFFSET_FILL,
		SAMPLE_ALPHA_TO_COVERAGE,
		SAMPLE_COVERAGE,
		SCISSOR_TEST,
		STENCIL_TEST,
#ifndef USING_GLES
		MULTISAMPLE
#endif
	};
	enum class RasterizationMode {
		POINT = 0,
		LINE,
		FILL
	};
	enum class ComparaisonAlgorithm {
		NEVER = 0,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOTEQUAL,
		GREATER_EQUAL,
		ALWAYS
	};

	enum class Operation {
		KEEP = 0,
		ZERO,
		REPLACE,
		INCREMENT,
		INCREMENT_WRAP,
		DECREMENT,
		DECREMENT_WRAP,
		INVERT
	};
	enum class CullFace {
		FRONT = 0,
		BACK,
		FRONT_AND_BACK
	};
	
	enum class PixelDataType {
		BYTE = 0,
		UNSIGNED_BYTE,
#ifndef USING_GLES
		BITMAP,
#endif
		SHORT,
		UNSIGNED_SHORT,
		INT,
		UNSIGNED_INT,
		FLOAT,
#ifndef USING_GLES
		UNSIGNED_BYTE_3_3_2,
		UNSIGNED_BYTE_2_3_3_REV,
		UNSIGNED_SHORT_5_6_5,
		UNSIGNED_SHORT_5_6_5_REV,
		UNSIGNED_SHORT_4_4_4_4,
		UNSIGNED_SHORT_4_4_4_4_REV,
		UNSIGNED_SHORT_5_5_5_1,
		UNSIGNED_SHORT_1_5_5_5_REV,
		UNSIGNED_INT_8_8_8_8,
		UNSIGNED_INT_8_8_8_8_REV,
		UNSIGNED_INT_10_10_10_2,
		UNSIGNED_INT_2_10_10_10_REV
#endif
	};
	enum class PixelDataFormat {
#ifndef USING_GLES
		COLOR_INDEX = 0,
		STENCIL_INDEX,
#endif
		DEPTH_COMPONENT,
#ifndef USING_GLES
		RED,
		GREEN,
		BLUE,
#endif
		ALPHA,
		RGB,
#ifndef USING_GLES
		BGR,
#endif
		RGBA,
#ifndef USING_GLES
		BGRA,
#endif
		LUMINANCE,
		LUMINANCE_ALPHA
	};

	enum class CullingOptions {
		NONE = 0,
		FRUSTUM_PER_MODEL,
		FRUSTUM_PER_MESH
	};
	inline CullingOptions operator~ (CullingOptions a) { return (CullingOptions)~(int)a; }
	inline CullingOptions operator| (CullingOptions a, CullingOptions b) { return (CullingOptions)((int)a | (int)b); }
	inline CullingOptions operator& (CullingOptions a, CullingOptions b) { return (CullingOptions)((int)a & (int)b); }
	inline CullingOptions operator^ (CullingOptions a, CullingOptions b) { return (CullingOptions)((int)a ^ (int)b); }
	inline CullingOptions& operator|= (CullingOptions& a, CullingOptions b) { return (CullingOptions&)((int&)a |= (int)b); }
	inline CullingOptions& operator&= (CullingOptions& a, CullingOptions b) { return (CullingOptions&)((int&)a &= (int)b); }
	inline CullingOptions& operator^= (CullingOptions& a, CullingOptions b) { return (CullingOptions&)((int&)a ^= (int)b); }
	inline bool isFlagSet(CullingOptions flag, CullingOptions mask) { return (int)flag & (int)mask; }


	enum class BlendFactor {
		NONE = 0,
		ZERO,
		ONE,
		ONE_MINUS_SRC_COLOR,
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA
	};

	enum class DepthFunction {
		EQUAL = 0,
		NOT_EQUAL,
		LESS,
		GREATER,
		LESS_EQUAL,
		GREATER_EQUAL,
		ALWAYS,
		NEVER
	};

}
