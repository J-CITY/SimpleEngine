#include "textureGl.h"

#include <array>
#include <vector>


#ifdef OPENGL_BACKEND
#include <iostream>
#include <gl/glew.h>
#include "../../../resourceManager/textureManager.h"

using namespace KUMA;
using namespace KUMA::RENDER;

std::shared_ptr<TextureGl> TextureGl::create(std::string path) {
    auto tex = std::make_shared<TextureGl>();

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width=0, height=0, nrComponents=0;

    
    //stbi_set_flip_vertically_on_load(true);
    unsigned char* data = KUMA::RESOURCES::stbiLoad(path.c_str(), &width, &height, &nrComponents, 0);
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
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        KUMA::RESOURCES::stbiImageFree(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        KUMA::RESOURCES::stbiImageFree(data);
    }

    tex->id = textureID;
    tex->mPath = path;
    tex->width = width;
    tex->height = height;
    return tex;
}

std::shared_ptr<TextureGl> TextureGl::CreateHDR(const std::string& path) {
	RESOURCES::stbiSetFlipVerticallyOnLoad(true);
    int width, height, nrComponents;
    float* data = RESOURCES::stbiLoadf(path.c_str(), &width, &height, &nrComponents, 0);
    unsigned int hdrTexture;
    if (data) {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

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

    auto tex = std::make_shared<TextureGl>();
    tex->id = texId;
    tex->width = texWidth;
    tex->height = texHeight;
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
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

    auto tex = std::make_shared<TextureGl>();
    tex->id = id;
    tex->type = TextureType::TEXTURE_2D_ARRAY;
    tex->width = texWidth;
    tex->height = texHeight;
    return tex;
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
        data = KUMA::RESOURCES::stbiLoad(path[i].c_str(), &width, &height, &nrChannels, 0);
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
