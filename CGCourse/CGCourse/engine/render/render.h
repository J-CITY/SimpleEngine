#pragma once
#include <string>
#include "../glManager/glManager.h"

namespace KUMA {
	namespace RESOURCES {
		class Mesh;
		//class Animator;
	}
	namespace RENDER {
		class Material;
		class Frustum;
		class Model;
		class Camera;

		enum class PrimitiveMode {
			POINTS = 0x0000,
			LINES = 0x0001,
			LINE_LOOP = 0x0002,
			LINE_STRIP = 0x0003,
			TRIANGLES = 0x0004,
			TRIANGLE_STRIP = 0x0005,
			TRIANGLE_FAN = 0x0006,
			LINES_ADJACENCY = 0x000A,
			LINE_STRIP_ADJACENCY = 0x000B,
			TRIANGLES_ADJACENCY = 0x000C,
			TRIANGLE_STRIP_ADJACENCY = 0x000D,
			PATCHES = 0xE,
		};
		enum class RenderingCapability {
			BLEND = 0x0BE2, // If enabled, blend the computed fragment color values with the values in the color buffers. See glBlendFunc.
			CULL_FACE = 0x0B44, // If enabled, cull polygons based on their winding in window coordinates.
			DEPTH_TEST = 0x0B71, // If enabled, do depth comparisons and update the depth buffer. Note that even if the depth buffer exists and the depth mask is non-zero, the depth buffer is not updated if the depth test is disabled.
			DITHER = 0x0BD0, // If enabled, dither color components or indices before they are written to the color buffer.
			POLYGON_OFFSET_FILL = 0x8037, // If enabled, an offset is added to depth values of a polygon's fragments produced by rasterization.
			SAMPLE_ALPHA_TO_COVERAGE = 0x809E, // If enabled, compute a temporary coverage value where each bit is determined by the alpha value at the corresponding sample location. The temporary coverage value is then ANDed with the fragment coverage value.
			SAMPLE_COVERAGE = 0x80A0, // If enabled, the fragment's coverage is ANDed with the temporary coverage value. If GL_SAMPLE_COVERAGE_INVERT is set to GL_TRUE, invert the coverage value.
			SCISSOR_TEST = 0x0C11, // If enabled, discard fragments that are outside the scissor rectangle.
			STENCIL_TEST = 0x0B90, // If enabled, do stencil testing and update the stencil buffer.
			MULTISAMPLE = 0x809D  // If enabled, use multiple fragment samples in computing the final color of a pixel.	
		};
		enum class RasterizationMode {
			POINT = 0x1B00, // Polygon vertices that are marked as the start of a boundary edge are drawn as points. Point attributes such as GL_POINT_SIZE and GL_POINT_SMOOTH control the rasterization of the points. Polygon rasterization attributes other than GL_POLYGON_MODE have no effect.
			LINE = 0x1B01, // Boundary edges of the polygon are drawn as line segments. Line attributes such as GL_LINE_WIDTH and GL_LINE_SMOOTH control the rasterization of the lines. Polygon rasterization attributes other than GL_POLYGON_MODE have no effect.
			FILL = 0x1B02  // The interior of the polygon is filled. Polygon attributes such as GL_POLYGON_SMOOTH control the rasterization of the polygon.
		};
		enum class ComparaisonAlgorithm {
			NEVER = 0x0200,
			LESS = 0x0201,
			EQUAL = 0x0202,
			LESS_EQUAL = 0x0203,
			GREATER = 0x0204,
			NOTEQUAL = 0x0205,
			GREATER_EQUAL = 0x0206,
			ALWAYS = 0x0207
		};

		enum class Operation {
			KEEP = 0x1E00,
			ZERO = 0,
			REPLACE = 0x1E01,
			INCREMENT = 0x1E02,
			INCREMENT_WRAP = 0x8507,
			DECREMENT = 0x1E03,
			DECREMENT_WRAP = 0x8508,
			INVERT = 0x150A
		};
		enum class CullFace {
			FRONT = 0x0404,
			BACK = 0x0405,
			FRONT_AND_BACK = 0x0408
		};
		enum class PixelDataType {
			BYTE = 0x1400,
			UNSIGNED_BYTE = 0x1401,
			BITMAP = 0x1A00,
			SHORT = 0x1402,
			UNSIGNED_SHORT = 0x1403,
			INT = 0x1404,
			UNSIGNED_INT = 0x1405,
			FLOAT = 0x1406,
			UNSIGNED_BYTE_3_3_2 = 0x8032,
			UNSIGNED_BYTE_2_3_3_REV = 0x8362,
			UNSIGNED_SHORT_5_6_5 = 0x8363,
			UNSIGNED_SHORT_5_6_5_REV = 0x8364,
			UNSIGNED_SHORT_4_4_4_4 = 0x8033,
			UNSIGNED_SHORT_4_4_4_4_REV = 0x8365,
			UNSIGNED_SHORT_5_5_5_1 = 0x8034,
			UNSIGNED_SHORT_1_5_5_5_REV = 0x8366,
			UNSIGNED_INT_8_8_8_8 = 0x8035,
			UNSIGNED_INT_8_8_8_8_REV = 0x8367,
			UNSIGNED_INT_10_10_10_2 = 0x8036,
			UNSIGNED_INT_2_10_10_10_REV = 0x8368
		};
		enum class PixelDataFormat {
			COLOR_INDEX = 0x1900,
			STENCIL_INDEX = 0x1901,
			DEPTH_COMPONENT = 0x1902,
			RED = 0x1903,
			GREEN = 0x1904,
			BLUE = 0x1905,
			ALPHA = 0x1906,
			RGB = 0x1907,
			BGR = 0x80E0,
			RGBA = 0x1908,
			BGRA = 0x80E1,
			LUMINANCE = 0x1909,
			LUMINANCE_ALPHA = 0x190A,
		};

		enum class CullingOptions {
			NONE = 0x0,
			FRUSTUM_PER_MODEL = 0x1,
			FRUSTUM_PER_MESH = 0x2
		};

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
			ONE_MINUS_CONSTANT_ALPHA,
		};
		

		enum class DepthFunction {
			EQUAL = 0,
			NOT_EQUAL,
			LESS,
			GREATER,
			LESS_EQUAL,
			GREATER_EQUAL,
			ALWAYS,
			NEVER,
		};

		
		
		inline CullingOptions operator~ (CullingOptions a) { return (CullingOptions)~(int)a; }
		inline CullingOptions operator| (CullingOptions a, CullingOptions b) { return (CullingOptions)((int)a | (int)b); }
		inline CullingOptions operator& (CullingOptions a, CullingOptions b) { return (CullingOptions)((int)a & (int)b); }
		inline CullingOptions operator^ (CullingOptions a, CullingOptions b) { return (CullingOptions)((int)a ^ (int)b); }
		inline CullingOptions& operator|= (CullingOptions& a, CullingOptions b) { return (CullingOptions&)((int&)a |= (int)b); }
		inline CullingOptions& operator&= (CullingOptions& a, CullingOptions b) { return (CullingOptions&)((int&)a &= (int)b); }
		inline CullingOptions& operator^= (CullingOptions& a, CullingOptions b) { return (CullingOptions&)((int&)a ^= (int)b); }
		inline bool isFlagSet(CullingOptions flag, CullingOptions mask) { return (int)flag & (int)mask; }
		
		class BaseRender {
		public:
			struct FrameInfo {
				uint64_t batchCount = 0;
				uint64_t instanceCount = 0;
				uint64_t polyCount = 0;
			};

			BaseRender(GL_SYSTEM::GlManager& driver);
			~BaseRender() = default;
			void setClearColor(float red, float green, float blue, float alpha = 1.0f);
			void clear(bool colorBuffer = true, bool depthBuffer = true, bool stencilBuffer = true);
			void setRasterizationLinesWidth(float width);
			void setRasterizationMode(RasterizationMode rasterizationMode);
			void setCapability(RenderingCapability capability, bool value);
			[[nodiscard]] bool getCapability(RenderingCapability capability) const;
			void setStencilAlgorithm(ComparaisonAlgorithm algorithm, int32_t reference, uint32_t mask);
			void setDepthAlgorithm(ComparaisonAlgorithm algorithm);
			void setStencilMask(uint32_t mask);
			void setStencilOperations(Operation stencilFail = Operation::KEEP, Operation depthFail = Operation::KEEP, 
				Operation bothPass = Operation::KEEP);
			void setCullFace(CullFace cullFace);
			void setDepthWriting(bool enable);
			void setColorWriting(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha);
			void setColorWriting(bool enable);
			void setViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
			void readPixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, PixelDataFormat format, PixelDataType type, void* data);
			bool getBool(GLenum parameter);
			bool getBool(GLenum parameter, uint32_t index);
			int getInt(GLenum parameter);
			int getInt(GLenum parameter, uint32_t index);
			float getFloat(GLenum parameter);
			float getFloat(GLenum parameter, uint32_t index);
			double getDouble(GLenum parameter);
			double getDouble(GLenum parameter, uint32_t index);
			int64_t getInt64(GLenum parameter);
			int64_t getInt64(GLenum parameter, uint32_t index);
			std::string getString(GLenum parameter);
			std::string getString(GLenum parameter, uint32_t index);
			void clearFrameInfo();
			void draw(const RESOURCES::Mesh & mesh, PrimitiveMode primitiveMode = PrimitiveMode::TRIANGLES, uint32_t instances = 1);
			uint8_t fetchGLState();
			void applyStateMask(uint8_t mask);
			void setState(uint8_t state);
			[[nodiscard]] const FrameInfo& getFrameInfo() const;
			void useDepthBufferMask(bool value);
			void useBlendFactors(BlendFactor src, BlendFactor dist);
			void useReversedDepth(bool value);
			void useDepthFunction(DepthFunction function);
			void useCulling(bool value, bool counterClockWise, bool cullBack);
			void drawIndices(PrimitiveMode primitive, size_t indexCount, size_t indexOffset);
			void drawIndicesBaseVertex(PrimitiveMode primitive, size_t indexCount, size_t indexOffset, size_t baseVertex);
			void drawIndicesBaseVertexInstanced(PrimitiveMode primitive, size_t indexCount, size_t indexOffset, size_t baseVertex, size_t instanceCount, size_t baseInstance);
			void setPatchSize(int sz) const;
		private:
			GL_SYSTEM::GlManager& driver;
			FrameInfo frameInfo;
			uint8_t state;
		};
	}
}
