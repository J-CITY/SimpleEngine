#pragma once

#include <array>
#include <map>
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
			~Texture() {
				glDeleteTextures(1, &id);
			};

			void setFilter(TextureFiltering minFilter, TextureFiltering magFilter);
			void setWrapType(TextureWrap wrapS, TextureWrap wrapT);
			void bind(uint32_t slot = 0) const;
			void bindWithoutAttach() const;
			void unbind() const;
			void generateMipmaps() const;
			void loadDepth(int _width, int _height);

			static void CopyTexture(const Texture & from, const Texture & to);

			//bool IsFloatingPoint() const {
			//	switch (this->format) {
			//	case TextureFormat::R:
			//		return false;
			//	case TextureFormat::R16:
			//		return false;
			//	case TextureFormat::RG:
			//		return false;
			//	case TextureFormat::RG16:
			//		return false;
			//	case TextureFormat::R16F:
			//		return true;
			//	case TextureFormat::R32F:
			//		return true;
			//	case TextureFormat::RG16F:
			//		return true;
			//	case TextureFormat::RG32F:
			//		return true;
			//	case TextureFormat::RGB:
			//		return false;
			//	case TextureFormat::RGBA:
			//		return false;
			//	case TextureFormat::RGB16:
			//		return false;
			//	case TextureFormat::RGB16F:
			//		return true;
			//	case TextureFormat::RGBA16:
			//		return false;
			//	case TextureFormat::RGBA16F:
			//		return true;
			//	case TextureFormat::RGB32F:
			//		return true;
			//	case TextureFormat::RGBA32F:
			//		return true;
			//	case TextureFormat::DEPTH:
			//		return false;
			//	case TextureFormat::DEPTH32F:
			//		return true;
			//	default:
			//		return false;
			//	}
			//}

			void load(uint8_t* data, int width, int height, int channels, bool isFloating, TextureFormat format) {
				this->path = "raw";
				this->width = width;
				this->height = height;
				this->textureType = GL_TEXTURE_2D;
				this->format = format;
			
				GLenum type = isFloating ? GL_FLOAT : GL_UNSIGNED_BYTE;
			
				GLenum dataChannels = GL_RGB;
				switch (channels) {
				case 1:
					dataChannels = GL_RED;
					break;
				case 2:
					dataChannels = GL_RG;
					break;
				case 3:
					dataChannels = GL_RGB;
					break;
				case 4:
					dataChannels = GL_RGBA;
					break;
				default:
					break;
				}
			
				glBindTexture(GL_TEXTURE_2D, id);
				glTexImage2D(GL_TEXTURE_2D, 0, formatTable[(int)this->format],
					(GLsizei)width, (GLsizei)height, 0, dataChannels, type, data);
			
				generateMipmaps();
			}
			uint32_t getId() const {
				return id;
			}

			uint32_t getWidth() const {
				return width;
			}

			uint32_t getHeight() const {
				return height;
			}

			uint32_t getType() const {
				return textureType;
			}

			std::string getPath() const {
				return path;
			}

			//void SetWrapType(TextureWrap wrapType) {
			//	bind(0);
			//	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrapTable[(int)wrapType]);
			//	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &wrapTable[(int)wrapType]);
			//}
			//void Texture::SetMaxLOD(size_t lod) {
			//	bind(0);
			//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, (float)lod);
			//}
			//size_t Texture::GetMaxTextureLOD() const {
			//	return log2(width > height ? width : height);
			//}
			//void Texture::SetMinLOD(size_t lod) {
			//	bind(0);
			//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, (float)lod);
			//}
		public:
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
			mutable uint32_t boundId = 0;
		};

		class CubeMap {
			friend class TextureLoader;
		public:
			const std::vector<GLenum> types = {GL_TEXTURE_CUBE_MAP_POSITIVE_X,
								  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
								  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
								  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
								  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
								  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
		public:
			mutable uint32_t BoundId = 0;
			CubeMap() {
				glGenTextures(1, &id);
			}
			CubeMap(const std::string path) :
				path(path) {
				glGenTextures(1, &id);
			}
			CubeMap(CubeMap& other) noexcept {
				this->id = other.id;
				//this->id = other.id;
				this->width = other.width;
				this->height = other.height;
				this->channels = other.channels;

				//other.id = 0;
				////other.active = 0;
				//other.width = 0;
				//other.height = 0;
				//other.channels = 0;
			}
			~CubeMap() = default;

			uint32_t getWidth() const {
				return width;
			}

			uint32_t getHeight() const {
				return height;
			}
			void bind(uint32_t slot = 0) const {
				glActiveTexture(GL_TEXTURE0 + slot);
				glBindTexture(GL_TEXTURE_CUBE_MAP, id);
				BoundId = slot;
			}


			void unbind() const {
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			void LoadDepth(int width, int height) {
				this->width = width;
				this->height = height;
				this->path = "depth";
				this->channels = 1;

				glBindTexture(GL_TEXTURE_CUBE_MAP, id);

				for (unsigned int i = 0; i < 6; ++i)
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				/*for (size_t i = 0; i < 6; i++) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, GL_DEPTH_COMPONENT,
						width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
				}

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

				float border [] = {1.0f, 1.0f, 1.0f, 1.0f};
				glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, border);

				GenerateMipmaps();*/
			}
			void GenerateMipmaps() {
				bind(0);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			}

			void Load(const std::array<uint8_t*, 6>& data, size_t width, size_t height) {
				this->width = width;
				this->height = height;
				this->channels = 3;
				this->path = "raw";
				glBindTexture(GL_TEXTURE_CUBE_MAP, id);
				for (size_t i = 0; i < data.size(); i++) {
					glTexImage2D(types[i], 0, GL_RGB,
						(GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data[i]);
				}

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				this->GenerateMipmaps();
			}
		public:
			uint32_t id = 0;
			int channels = 1;
			std::string path;
			uint32_t width = 0;
			uint32_t height = 0;
		};


	}
}
