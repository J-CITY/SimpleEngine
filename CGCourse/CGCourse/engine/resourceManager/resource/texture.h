#pragma once

#include <array>
#include <optional>
#include <GL/glew.h>
#include <string>
#include <vector>


namespace KUMA {
	namespace RESOURCES {
		class TextureLoader;
		enum class TextureFormat : uint8_t {
			R,
			R16,
			RG,
			RG16,
			R16F,
			R32F,
			RG16F,
			RG32F,
			RGB,
			RGBA,
			RGB16,
			RGB16F,
			RGBA16,
			RGBA16F,
			RGB32F,
			RGBA32F,
			DEPTH,
			DEPTH32F
		};
		enum class TextureWrap : uint8_t {
			CLAMP_TO_EDGE,
			CLAMP_TO_BORDER,
			MIRRORED_REPEAT,
			REPEAT,
		};
		enum class TextureFiltering {
			NEAREST = 0x2600,
			LINEAR = 0x2601,
			NEAREST_MIPMAP_NEAREST = 0x2700,
			LINEAR_MIPMAP_LINEAR = 0x2703,
			LINEAR_MIPMAP_NEAREST = 0x2701,
			NEAREST_MIPMAP_LINEAR = 0x2702
		};

		class Texture {
			friend class TextureLoader;
			std::vector<GLenum> formatTable = {
				GL_R8,
				GL_R16,
				GL_RG8,
				GL_RG16,
				GL_R16F,
				GL_R32F,
				GL_RG16F,
				GL_RG32F,
				GL_RGB,
				GL_RGBA,
				GL_RGB16,
				GL_RGB16F,
				GL_RGBA16,
				GL_RGBA16F,
				GL_RGB32F,
				GL_RGBA32F,
				GL_DEPTH_COMPONENT,
				GL_DEPTH_COMPONENT32F
			};

			std::vector<GLint> wrapTable = {
					GL_CLAMP_TO_EDGE,
					GL_CLAMP_TO_BORDER,
					GL_MIRRORED_REPEAT,
					GL_REPEAT,
			};
		public:
			Texture();
			Texture(const std::string path, uint32_t id, uint32_t width, uint32_t height, uint32_t bpp, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			Texture(unsigned w, unsigned h) noexcept;
			~Texture();;

			void setFilter(TextureFiltering minFilter, TextureFiltering magFilter);
			void setWrapType(TextureWrap wrapS, TextureWrap wrapT);
			void bind(uint32_t slot = 0) const;
			void bindWithoutAttach() const;
			void unbind() const;
			void generateMipmaps() const;
			void loadDepth(int _width, int _height);

			static void CopyTexture(const Texture & from, const Texture & to);
			
			void load(uint8_t* data, int width, int height, int channels, bool isFloating, TextureFormat format);
			uint32_t getId() const;
			uint32_t getWidth() const;
			uint32_t getHeight() const;
			uint32_t getType() const;
			std::string getPath() const;
		private:
			uint32_t id = -1;
			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t bitsPerPixel = 0;
			TextureFiltering firstFilter = TextureFiltering::LINEAR;
			TextureFiltering secondFilter = TextureFiltering::LINEAR;
			std::string path;
			bool isMimapped = false;
			TextureFormat format;
			unsigned int textureType = 0;
			mutable std::optional<uint32_t> boundId = std::nullopt;
		};

		class CubeMap {
			friend class TextureLoader;
		private:
			const std::vector<GLenum> types = {GL_TEXTURE_CUBE_MAP_POSITIVE_X,
								  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
								  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
								  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
								  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
								  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
		public:
			CubeMap();

			CubeMap(const std::string path);

			CubeMap(CubeMap& other) noexcept;
			~CubeMap() = default;

			uint32_t getId() const;
			uint32_t getWidth() const;
			uint32_t getHeight() const;
			void bind(uint32_t slot = 0) const;
			void bindWithoutAttach() const;
			void unbind() const;
			void LoadDepth(int width, int height);
			void generateMipmaps() const;
			void Load(const std::array<uint8_t*, 6>& data, size_t width, size_t height);
		private:
			uint32_t id = 0;
			int channels = 1;
			std::string path;
			uint32_t width = 0;
			uint32_t height = 0;
			mutable std::optional<uint32_t> boundId = std::nullopt;
		};


	}
}
