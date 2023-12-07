#include "textureGl.h"

#include <array>
#include <vector>


#ifdef OPENGL_BACKEND
#include <iostream>
#include <gl/glew.h>
#include <coreModule/resourceManager/textureManager.h>
#include <renderModule/backends/interface/resourceStruct.h>

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

TextureGl::~TextureGl() {
    glDeleteTextures(1, &id);
}

enum class TextureType
{
	TEX2D = 0x0DE1,
    TEX3D = 0x806F,
    CUBE = 0x8513,
    ARRAY2D = 0x8C1A,
};

enum class TextureFormat
{
    AUTO,
    MONO = 0x1903,
    RGB = 0x1907,
    RGBA = 0x1908
};

enum class TextureWrap
{
    CLAMP = 0x2900,
    REPEAT = 0x2901
};

enum class TextureFilter
{
    LINEAR = 0x2601,
    NEAREST = 0x2600,
    LINEAR_MIPMAP_LINEAR = 0x2703

};

std::shared_ptr<TextureGl> TextureGl::Create(std::string path, bool generateMipmap) {
    auto tex = std::make_shared<TextureGl>();

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width=0, height=0, nrComponents=0;

    //IKIGAI::RESOURCES::stbiSetFlipVerticallyOnLoad(true);
    //stbi_set_flip_vertically_on_load(true);
    unsigned char* data = IKIGAI::RESOURCES::stbiLoad(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        if (generateMipmap) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        IKIGAI::RESOURCES::stbiImageFree(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        IKIGAI::RESOURCES::stbiImageFree(data);
    }

    tex->id = textureID;
    tex->mPath = path;
    tex->width = width;
    tex->height = height;
    tex->chanels = nrComponents;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateFromResource(const RENDER::TextureResource& res) {
    int chanels = 4;
    auto getFormat = [](int nrComponents) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        return format;
    };
    auto getInternalFormat = [](int nrComponents, bool isFloat) {
        GLenum format;
        if (nrComponents == 1)
            format = isFloat ? GL_R16F : GL_RED;
        else if (nrComponents == 3)
            format = isFloat ? GL_RGB16F : GL_RGB;
        else if (nrComponents == 4)
            format = isFloat ? GL_RGBA16F : GL_RGBA;
        return format;
    };

    auto getFormat2 = [&](PixelDataFormat dataFormat) {
        GLenum format;
        if (dataFormat == PixelDataFormat::RED) {
            format = GL_RED;
            chanels = 1;
        }
        else if (dataFormat == PixelDataFormat::RGB) {
            format = GL_RGB;
            chanels = 3;
        }
        else if (dataFormat == PixelDataFormat::RGBA) {
            format = GL_RGBA;
            chanels = 4;
        }
        else if (dataFormat == PixelDataFormat::DEPTH_COMPONENT) {
            format = GL_DEPTH_COMPONENT;
            chanels = 1;
        }
        return format;
    };
    auto getInternalFormat2 = [&](PixelDataFormat dataFormat, bool isFloat) {
        GLenum format;
        if (dataFormat == PixelDataFormat::RED) {
            format = isFloat ? GL_R16F : GL_RED;
            chanels = 1;
        }
        else if (dataFormat == PixelDataFormat::RGB) {
            format = isFloat ? GL_RGB16F : GL_RGB;
            chanels = 3;
        }
        else if (dataFormat == PixelDataFormat::RGBA) {
            format = isFloat ? GL_RGBA16F : GL_RGBA;
            chanels = 4;
        }
        else if (dataFormat == PixelDataFormat::DEPTH_COMPONENT) {
            format = isFloat ? GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT;
            chanels = 1;
        }
        return format;
    };

    auto getType = [](TextureType type) {
        switch (type)
        {
        case TextureType::TEXTURE_2D: return GL_TEXTURE_2D;
        case TextureType::TEXTURE_3D: return GL_TEXTURE_3D;
        case TextureType::TEXTURE_CUBE: return GL_TEXTURE_CUBE_MAP;
        case TextureType::TEXTURE_2D_ARRAY: return GL_TEXTURE_2D_ARRAY;
        }
        return GL_TEXTURE_2D;
    };
    auto createTexture = [](TextureType type, int internalFormat, int format, int width, int height, int depth, bool isFloat, const void* data) {
        switch (type)
        {
        case TextureType::TEXTURE_2D:
	    {
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), data);
	    }
        return;
        case TextureType::TEXTURE_3D: 
        {
            glTexImage3D(
                GL_TEXTURE_3D, 0, internalFormat, width, height, depth,
                0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), data);
        }
        return;
        case TextureType::TEXTURE_CUBE:
        {
            for (unsigned int i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), data);
            }

	    }
        return;
        case TextureType::TEXTURE_2D_ARRAY: 
        {
            glTexImage3D(
                GL_TEXTURE_2D_ARRAY, 0, internalFormat, width, height, depth,
                0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), data);
        }
        return;
        }
    };


    auto tex = std::make_shared<TextureGl>();

    unsigned texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(getType(res.texType), texId);
    if (!res.path.empty()) {
        int width=0, height=0, nrComponents=0;
	    if (res.isFloat) {
            RESOURCES::stbiSetFlipVerticallyOnLoad(true);
            float* data = RESOURCES::stbiLoadf(res.path.c_str(), &width, &height, &nrComponents, 0);
            if (data) {
                createTexture(res.texType, getInternalFormat(nrComponents, res.isFloat), getFormat(nrComponents), width, height, res.depth, res.isFloat, data);
                RESOURCES::stbiImageFree(data);
            }
	    }
        else {
            unsigned char* data = IKIGAI::RESOURCES::stbiLoad(res.path.c_str(), &width, &height, &nrComponents, 0);
            if (data) {
                createTexture(res.texType, getInternalFormat(nrComponents, res.isFloat), getFormat(nrComponents), width, height, res.depth, res.isFloat, data);
                RESOURCES::stbiImageFree(data);
            }
        }
        tex->width = width;
        tex->height = height;
        tex->depth = res.depth;
        tex->chanels = chanels;
    }
    else {
        if (res.isFloat) {
            RESOURCES::stbiSetFlipVerticallyOnLoad(true);
        }
        const uint8_t* data = nullptr;
        if (res.useColor) {
            data = res.colorData.data();
        }
    	createTexture(res.texType, getInternalFormat2(res.pixelType, res.isFloat), getFormat2(res.pixelType), res.width, res.height, res.depth, res.isFloat, data);
        tex->width = res.width;
        tex->height = res.height;
        tex->depth = res.depth;
    }

    if (res.useMipmap) {
        glGenerateMipmap(getType(res.texType));
    }
    glBindTexture(getType(res.texType), 0);
    
    tex->id = texId;
    tex->mPath = res.path;
    
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateHDR(const std::string& path, bool generateMipmap) {
	RESOURCES::stbiSetFlipVerticallyOnLoad(true);
    int width, height, nrComponents;
    float* data = RESOURCES::stbiLoadf(path.c_str(), &width, &height, &nrComponents, 0);
    unsigned int hdrTexture;
    if (data) {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float
        if (generateMipmap) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        RESOURCES::stbiImageFree(data);
    }
    else {
        std::cout << "Failed to load HDR image." << std::endl;
    }

    auto tex = std::make_shared<TextureGl>();
    tex->id = hdrTexture;
    tex->width = width;
    tex->height = height;
    return tex;
}

void TextureGl::CopyTexture(const TextureGl& from, const TextureGl& to) {
    if (from.width != to.width || from.height != to.height) {
        throw std::logic_error("Textures have different size");
    }
    glCopyImageSubData(from.id, GL_TEXTURE_2D, 0, 0, 0, 0,
        to.id, GL_TEXTURE_2D, 0, 0, 0, 0,
        from.width, from.height, 1);
}

std::shared_ptr<TextureGl> TextureGl::CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap) {
    auto tex = std::make_shared<TextureGl>();

	GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    if (generateMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    tex->id = textureID;
    tex->width = width;
    tex->height = height;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::createForAttach(int texWidth, int texHeight, int type) {
    unsigned int texId;
    // position color buffer
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, texWidth, texHeight, 0, GL_RGBA, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateMipmap(GL_TEXTURE_2D);

    auto tex = std::make_shared<TextureGl>();
    tex->id = texId;
    tex->width = texWidth;
    tex->height = texHeight;
    tex->chanels = 4;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::createDepthForAttachCubemap(int texWidth, int texHeight, int type) {
    unsigned int texId;
   
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texId);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, type, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	auto tex = std::make_shared<TextureGl>();
    tex->id = texId;
    tex->width = texWidth;
    tex->height = texHeight;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::createDepthForAttach(unsigned texWidth, unsigned texHeight)
{
    unsigned int texId;
    // position color buffer
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    auto tex = std::make_shared<TextureGl>();
    tex->id = texId;
    tex->width = texWidth;
    tex->height = texHeight;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::createDepthForAttach2DArray(int texWidth, int texHeight, int arrSize) {
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, texWidth, texHeight, arrSize,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    //constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    //glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

    auto tex = std::make_shared<TextureGl>();
    tex->id = id;
    tex->type = TextureType::TEXTURE_2D_ARRAY;
    tex->width = texWidth;
    tex->height = texHeight;
    tex->depth = arrSize;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::createEmpty3d(int texX, int texY, int texZ) {
    int m_mip_levels = 1;
    int width = texX;
    int height = texY;
    int depth = texZ;

    while (width > 1 && height > 1 && depth > 1) {
        width = std::max(1, (width / 2));
        height = std::max(1, (height / 2));
        depth = std::max(1, (depth / 2));
        m_mip_levels++;
    }

    unsigned int id;
    glCreateTextures(GL_TEXTURE_3D, 1, &id);

    glTextureStorage3D(id, m_mip_levels, GL_RGBA16F, texX, texY, texZ);

    // Default sampling options.
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (m_mip_levels > 1)
        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    else
        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    auto tex = std::make_shared<TextureGl>();
    tex->id = id;
    tex->type = TextureType::TEXTURE_3D;
    tex->width = texX;
    tex->height = texY;
    tex->depth = texZ;
    return tex;
}

std::vector<unsigned char> TextureGl::getPixels(const std::string& path) {
    int width = 0, height = 0, nrComponents = 0;
    unsigned char* data = IKIGAI::RESOURCES::stbiLoad(path.c_str(), &width, &height, &nrComponents, 0);
    std::vector<unsigned char> res(data, data + width * height * nrComponents);
    IKIGAI::RESOURCES::stbiImageFree(data);
    return res;
}

void TextureGl::bind(int _slot) {
    slot = _slot;
    glActiveTexture(GL_TEXTURE0 + slot);
    if (type == TextureType::TEXTURE_2D) {
        glBindTexture(GL_TEXTURE_2D, id);
    }
    else if (type == TextureType::TEXTURE_CUBE) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    }
    else if (type == TextureType::TEXTURE_2D_ARRAY) {
        glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    }
    else if (type == TextureType::TEXTURE_3D) {
        glBindTexture(GL_TEXTURE_3D, id);
    }
}

void TextureGl::unbind() {
    glActiveTexture(GL_TEXTURE0);
}

void TextureGl::generateMipmaps() {
    if (type == TextureType::TEXTURE_2D) {
        glBindTexture(GL_TEXTURE_2D, id);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else if (type == TextureType::TEXTURE_CUBE) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }
    else if (type == TextureType::TEXTURE_3D) {
        glBindTexture(GL_TEXTURE_3D, id);
        glGenerateMipmap(GL_TEXTURE_3D);
    }
}

void TextureGl::bindImage(uint32_t unit, uint32_t mip_level, uint32_t layer, unsigned access, unsigned format) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_3D, id);

	if (type == TextureType::TEXTURE_3D)
		glBindImageTexture(unit, id, mip_level, GL_TRUE, layer, access, format);
	else
		glBindImageTexture(unit, id, mip_level, GL_FALSE, 0, access, format);
}

std::shared_ptr<TextureGl> TextureGl::CreateHDREmptyCubemap(int width, int height) {
    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto tex = std::make_shared<TextureGl>();
    tex->id = envCubemap;
    tex->type = TextureType::TEXTURE_CUBE;
    tex->width = width;
    tex->height = height;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::createCubemap(std::array<std::string, 6> path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);


    int width, height, nrChannels;
    unsigned char* data;
    for (unsigned int i = 0; i < path.size(); i++)
    {
        data = IKIGAI::RESOURCES::stbiLoad(path[i].c_str(), &width, &height, &nrChannels, 0);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    auto tex = std::make_shared<TextureGl>();
    tex->id = textureID;
    tex->type = TextureType::TEXTURE_CUBE;
    return tex;
}
#endif
