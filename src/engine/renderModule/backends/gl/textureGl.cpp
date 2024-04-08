#include "textureGl.h"

#include <array>
#include <filesystem>
#include <vector>

#include "utilsModule/jsonLoader.h"
#include "utilsModule/pathGetter.h"
#include "utilsModule/stdLoader.h"

//#include "coreModule/ecs/components/transform.h"


#ifdef OPENGL_BACKEND
#include <iostream>
#include <coreModule/graphicsWrapper.hpp>
//#include <coreModule/resourceManager/textureManager.h>
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


std::shared_ptr<TextureGl> TextureGl::Create(const std::string& path, bool generateMipmap) {
    auto tex = std::make_shared<TextureGl>();

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width=0, height=0, nrComponents=0;

    IKIGAI::UTILS::STBiSetFlipVerticallyOnLoad(true);
    //stbi_set_flip_vertically_on_load(true);
    unsigned char* data = IKIGAI::UTILS::STBiLoad(UTILS::GetRealPath(path).c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGBA;
        if (nrComponents == 1)
#ifndef USING_GLES
            format = GL_RED;
#else
            format = GL_ALPHA;
#endif
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        if (generateMipmap) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); // Use GL_NEAREST_MIPMAP_LINEAR if you want to use mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        IKIGAI::UTILS::STBiImageFree(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        IKIGAI::UTILS::STBiImageFree(data);
    }

    tex->id = textureID;
    tex->mPath = path;
    tex->mWidth = width;
    tex->mHeight = height;
    tex->mChannels = nrComponents;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateFromResource(const RENDER::TextureResource& res) {
    int chanels = 4;
    auto getFormat = [](int nrComponents) {
        GLenum format = GL_RGBA;
        if (nrComponents == 1)
#ifndef USING_GLES
            format = GL_RED;
#else
            format = GL_ALPHA;
#endif
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        return format;
    };
    auto getInternalFormat = [](int nrComponents, bool isFloat) {
        GLenum format = GL_RGBA;
        if (nrComponents == 1)
#ifndef USING_GLES
            format = isFloat ? GL_R16F : GL_RED;
#else
            format = GL_ALPHA;
#endif
        else if (nrComponents == 3)
#ifndef USING_GLES
            format = isFloat ? GL_RGB16F : GL_RGB;
#else
            format = GL_RGB;
#endif
        else if (nrComponents == 4)
#ifndef USING_GLES
            format = isFloat ? GL_RGBA16F : GL_RGBA;
#else
            format = GL_RGBA;
#endif
        return format;
    };

    auto getFormat2 = [&](PixelDataFormat dataFormat) {
        GLenum format = GL_RGBA;
#ifndef USING_GLES
        if (dataFormat == PixelDataFormat::RED) {
            format = GL_RED;
            chanels = 1;
        }
#else
        if (dataFormat == PixelDataFormat::ALPHA) {
            format = GL_ALPHA;
            chanels = 1;
        }
#endif
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
        GLenum format = GL_RGBA;
#ifndef USING_GLES
        if (dataFormat == PixelDataFormat::RED) {
            format = isFloat ? GL_R16F : GL_RED;
            chanels = 1;
        }
#else
        if (dataFormat == PixelDataFormat::ALPHA) {
            format = GL_ALPHA;
            chanels = 1;
        }
#endif
        else if (dataFormat == PixelDataFormat::RGB) {
#ifndef USING_GLES
            format = isFloat ? GL_RGB16F : GL_RGB;
#else
            format = GL_RGB;
#endif
            chanels = 3;
        }
        else if (dataFormat == PixelDataFormat::RGBA) {
#ifndef USING_GLES
            format = isFloat ? GL_RGBA16F : GL_RGBA;
#else
            format = GL_RGBA;
#endif
            chanels = 4;
        }
        else if (dataFormat == PixelDataFormat::DEPTH_COMPONENT) {
#ifndef USING_GLES
            format = isFloat ? GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT;
#else
            format = GL_DEPTH_COMPONENT;
#endif
            chanels = 1;
        }
        return format;
    };

    auto getType = [](TextureType type) {
        switch (type)
        {
        case TextureType::TEXTURE_2D: return GL_TEXTURE_2D;
#ifndef USING_GLES
        case TextureType::TEXTURE_3D: return GL_TEXTURE_3D;
#endif
        case TextureType::TEXTURE_CUBE: return GL_TEXTURE_CUBE_MAP;
#ifndef USING_GLES
        case TextureType::TEXTURE_2D_ARRAY: return GL_TEXTURE_2D_ARRAY;
#endif
        default: return GL_TEXTURE_2D;
        }
        return GL_TEXTURE_2D;
    };
    auto createTexture = [](TextureType type, int internalFormat, int format, int width, int height, int depth, bool isFloat, std::vector<void*>& datas) {
        switch (type)
        {
        case TextureType::TEXTURE_2D:
	    {
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), datas[0]);
	    }
        return;
        case TextureType::TEXTURE_3D: 
        {
#ifndef USING_GLES
            glTexImage3D(
                GL_TEXTURE_3D, 0, internalFormat, width, height, depth,
                0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), datas[0]);
#endif
        }
        return;
        case TextureType::TEXTURE_CUBE:
        {
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            for (unsigned int i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), datas[i]);
            }

	    }
        return;
        case TextureType::TEXTURE_2D_ARRAY: 
        {
#ifndef USING_GLES
            glTexImage3D(
                GL_TEXTURE_2D_ARRAY, 0, internalFormat, width, height, depth,
                0, format, (isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE), datas[0]);
#endif
        }
        return;
        }
    };


    auto tex = std::make_shared<TextureGl>();

    unsigned texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(getType(res.texType), texId);
    if (!res.pathTexture.empty()) {
        UTILS::STBiSetFlipVerticallyOnLoad(true);
        int width=0, height=0, nrComponents=0;
	    if (res.isFloat) {
            std::vector<void*> datas;
            for (auto& p : res.pathTexture) {
                float* data = UTILS::STBiLoadf(UTILS::GetRealPath(p).c_str(), &width, &height, &nrComponents, 0);
                if (!data) {

                }
                datas.push_back(data);
            }
	    	createTexture(res.texType, getInternalFormat(nrComponents, res.isFloat), getFormat(nrComponents), width, height, res.depth, res.isFloat, datas);
	    	for (auto d : datas) {
	    		UTILS::STBiImageFree((float*)d);
	    	}
	    }
        else {
            std::vector<void*> datas;
            for (auto& p : res.pathTexture) {
                unsigned char* data = UTILS::STBiLoad(UTILS::GetRealPath(p).c_str(), &width, &height, &nrComponents, 0);
                if (!data) {

                }
                datas.push_back(data);
            }
        	createTexture(res.texType, getInternalFormat(nrComponents, res.isFloat), getFormat(nrComponents), width, height, res.depth, res.isFloat, datas);
            for (auto d : datas) {
                UTILS::STBiImageFree((unsigned char*)d);
            }
        }
        tex->mWidth = width;
        tex->mHeight = height;
        tex->mDepth = res.depth;
        tex->mChannels = chanels;
    }
    else {
        if (res.isFloat) {
            UTILS::STBiSetFlipVerticallyOnLoad(true);
        }
        const uint8_t* data = nullptr;
        if (!res.colorData.empty()) {
            data = res.colorData.data();
        }
        std::vector<void*> datas;
        datas.push_back((void*)data);
    	createTexture(res.texType, getInternalFormat2(res.pixelType, res.isFloat), getFormat2(res.pixelType), res.width, res.height, res.depth, res.isFloat, datas);
        tex->mWidth = res.width;
        tex->mHeight = res.height;
        tex->mDepth = res.depth;
    }

    if (res.useMipmap) {
        glGenerateMipmap(getType(res.texType));
    }
    glBindTexture(getType(res.texType), 0);
    
    tex->id = texId;
    tex->mPath = res.path;
    tex->mType = res.texType;
    
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateHDR(const std::string& path, bool generateMipmap) {
	UTILS::STBiSetFlipVerticallyOnLoad(true);
    int width, height, nrComponents;
    float* data = UTILS::STBiLoadf(path.c_str(), &width, &height, &nrComponents, 0);
    unsigned int hdrTexture = 0;
    if (data) {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
#ifndef USING_GLES
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float
#endif
        if (generateMipmap) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        UTILS::STBiImageFree(data);
    }
    else {
        std::cout << "Failed to load HDR image." << std::endl;
    }

    auto tex = std::make_shared<TextureGl>();
    tex->id = hdrTexture;
    tex->mWidth = width;
    tex->mHeight = height;
    return tex;
}

void TextureGl::CopyTexture(const TextureGl& from, const TextureGl& to) {
    if (from.mWidth != to.mWidth || from.mHeight != to.mHeight) {
        throw std::logic_error("Textures have different size");
    }
#ifndef USING_GLES
    glCopyImageSubData(from.id, GL_TEXTURE_2D, 0, 0, 0, 0,
        to.id, GL_TEXTURE_2D, 0, 0, 0, 0,
        from.mWidth, from.mHeight, 1);
#endif
}

std::shared_ptr<TextureGl> TextureGl::CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap) {
    auto tex = std::make_shared<TextureGl>();

	GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
#ifndef USING_GLES
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
#endif
    if (generateMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    tex->id = textureID;
    tex->mWidth = width;
    tex->mHeight = height;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateForAttach(int texWidth, int texHeight, int type) {
    unsigned int texId;
    // position color buffer
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
#ifndef USING_GLES
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, texWidth, texHeight, 0, GL_RGBA, type, NULL);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
#endif
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    auto tex = std::make_shared<TextureGl>();
    tex->id = texId;
    tex->mWidth = texWidth;
    tex->mHeight = texHeight;
    tex->mChannels = 4;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateDepthForAttachCubemap(int texWidth, int texHeight, int type) {
    unsigned int texId;
   
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texId);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, type, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef USING_GLES
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif
	auto tex = std::make_shared<TextureGl>();
    tex->id = texId;
    tex->mWidth = texWidth;
    tex->mHeight = texHeight;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateDepthForAttach(unsigned texWidth, unsigned texHeight)
{
    unsigned int texId;
    // position color buffer
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#ifndef USING_GLES
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
#endif
    auto tex = std::make_shared<TextureGl>();
    tex->id = texId;
    tex->mWidth = texWidth;
    tex->mHeight = texHeight;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateDepthForAttach2DArray(int texWidth, int texHeight, int arrSize) {
    unsigned int id;
    glGenTextures(1, &id);
#ifndef USING_GLES
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, texWidth, texHeight, arrSize,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif
    //constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    //glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

    auto tex = std::make_shared<TextureGl>();
    tex->id = id;
    tex->mType = TextureType::TEXTURE_2D_ARRAY;
    tex->mWidth = texWidth;
    tex->mHeight = texHeight;
    tex->mDepth = arrSize;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateEmpty3d(int texX, int texY, int texZ) {
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

    unsigned int id=0;
#ifndef USING_GLES
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
#endif
    auto tex = std::make_shared<TextureGl>();
    tex->id = id;
    tex->mType = TextureType::TEXTURE_3D;
    tex->mWidth = texX;
    tex->mHeight = texY;
    tex->mDepth = texZ;
    return tex;
}

std::vector<unsigned char> TextureGl::GetPixels(const std::string& path) {
    int width = 0, height = 0, nrComponents = 0;
    unsigned char* data = IKIGAI::UTILS::STBiLoad(path.c_str(), &width, &height, &nrComponents, 0);
    std::vector<unsigned char> res(data, data + width * height * nrComponents);
    IKIGAI::UTILS::STBiImageFree(data);
    return res;
}

void TextureGl::bind(int _slot) {
    slot = _slot;
    glActiveTexture(GL_TEXTURE0 + slot);
    if (mType == TextureType::TEXTURE_2D) {
        glBindTexture(GL_TEXTURE_2D, id);
    }
    else if (mType == TextureType::TEXTURE_CUBE) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    }
    else if (mType == TextureType::TEXTURE_2D_ARRAY) {
#ifndef USING_GLES
        glBindTexture(GL_TEXTURE_2D_ARRAY, id);
#endif
    }
    else if (mType == TextureType::TEXTURE_3D) {
#ifndef USING_GLES
        glBindTexture(GL_TEXTURE_3D, id);
#endif
    }
}

void TextureGl::unbind() {
    glActiveTexture(GL_TEXTURE0);
}

void TextureGl::generateMipmaps() {
    if (mType == TextureType::TEXTURE_2D) {
        glBindTexture(GL_TEXTURE_2D, id);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else if (mType == TextureType::TEXTURE_CUBE) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }
    else if (mType == TextureType::TEXTURE_3D) {
#ifndef USING_GLES
        glBindTexture(GL_TEXTURE_3D, id);
        glGenerateMipmap(GL_TEXTURE_3D);
#endif
    }
}

void TextureGl::bindImage(uint32_t unit, uint32_t mip_level, uint32_t layer, unsigned access, unsigned format) {
    glActiveTexture(GL_TEXTURE0 + unit);
#ifndef USING_GLES
    glBindTexture(GL_TEXTURE_3D, id);

	if (mType == TextureType::TEXTURE_3D)
		glBindImageTexture(unit, id, mip_level, GL_TRUE, layer, access, format);
	else
		glBindImageTexture(unit, id, mip_level, GL_FALSE, 0, access, format);
#endif
}

void* TextureGl::getImguiId() {
	return reinterpret_cast<void*>(id);
}

std::shared_ptr<TextureGl> TextureGl::CreateHDREmptyCubemap(int width, int height) {
    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
#ifndef USING_GLES
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
#endif
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef USING_GLES
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto tex = std::make_shared<TextureGl>();
    tex->id = envCubemap;
    tex->mType = TextureType::TEXTURE_CUBE;
    tex->mWidth = width;
    tex->mHeight = height;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateCubemap(std::array<std::string, 6> path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);


    int width, height, nrChannels;
    unsigned char* data;
    for (unsigned int i = 0; i < path.size(); i++)
    {
        data = IKIGAI::UTILS::STBiLoad(path[i].c_str(), &width, &height, &nrChannels, 0);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef USING_GLES
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

    auto tex = std::make_shared<TextureGl>();
    tex->id = textureID;
    tex->mType = TextureType::TEXTURE_CUBE;
    return tex;
}

//---------------------------------------

AtlasRect TextureAtlas::getPiece(const std::string& name) const {
	if (mAtlas.mRects.contains(name)) {
		return mAtlas.mRects.at(name);
	}
	return AtlasRect();
}

AtlasRect TextureAtlas::getPieceUV(const std::string& name) const {
	if (mAtlas.mRects.contains(name)) {
		auto res = mAtlas.mRects.at(name);
		res.mX /= mWidth;
		res.mY /= mHeight;
		res.mW /= mWidth;
		res.mH /= mHeight;
		return res;
	}
	return AtlasRect();
}

std::shared_ptr<TextureAtlas> TextureAtlas::CreateAtlas(const std::string& path, bool generateMipmap) {
    auto tex = std::make_shared<TextureAtlas>();

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width = 0, height = 0, nrComponents = 0;

    //IKIGAI::UTILS::stbiSetFlipVerticallyOnLoad(true);
    //stbi_set_flip_vertically_on_load(true);
    unsigned char* data = UTILS::STBiLoad(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
#ifndef USING_GLES
            format = GL_RED;
#else
            format = GL_ALPHA;
#endif
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

        IKIGAI::UTILS::STBiImageFree(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        IKIGAI::UTILS::STBiImageFree(data);
    }

    tex->id = textureID;
    tex->mPath = path;
    tex->mWidth = width;
    tex->mHeight = height;
    tex->mChannels = nrComponents;

    std::filesystem::path configPath{ path };
    configPath.replace_extension(".atlas");

    //const std::string jsonData = UTILS::readFileIntoString(UTILS::getRealPath(configPath.string()));
    //auto json = nlohmann::json::parse(jsonData, nullptr, true, true);
    //
    //AtlasData adata;
    //adata.mPath = json["Path"];
    //for (auto& e : json["Files"]) {
    //    std::string key = e["key"];
    //    AtlasRect val(e["value"]["x"], e["value"]["y"], e["value"]["w"], e["value"]["h"]);
    //    adata.mRects.insert({ key, val });
    //}
    //tex->mAtlas = adata;

    //TODO: why is not work
    auto res = UTILS::FromJson<AtlasData>(configPath.string());
    if (res.isOk()) {
        tex->mAtlas = res.unwrap();
    }
    else {
        throw;
    }
    return tex;
}


#endif
