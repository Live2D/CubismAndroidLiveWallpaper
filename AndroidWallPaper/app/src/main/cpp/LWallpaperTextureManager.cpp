/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LWallpaperTextureManager.hpp"
#include <iostream>
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "LWallpaperPal.hpp"

LWallpaperTextureManager::LWallpaperTextureManager()
{
}

LWallpaperTextureManager::~LWallpaperTextureManager()
{
    ReleaseTextures();
}

LWallpaperTextureManager::TextureInfo* LWallpaperTextureManager::CreateTextureFromPngFile(std::string fileName)
{
    //search loaded texture already.
    for (Csm::csmUint32 i = 0; i < _textures.GetSize(); i++)
    {
        if (_textures[i]->fileName == fileName)
        {
            return _textures[i];
        }
    }

    GLuint textureId;
    int width, height, channels;
    unsigned int size;
    unsigned char* png;
    unsigned char* address;

    address = LWallpaperPal::LoadFileAsBytes(fileName, &size);

    // png情報を取得する
    png = stbi_load_from_memory(
        address,
        static_cast<int>(size),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha);
    {
#ifdef PREMULTIPLIED_ALPHA_ENABLE
        unsigned int* fourBytes = reinterpret_cast<unsigned int*>(png);
        for (int i = 0; i < width * height; i++)
        {
            unsigned char* p = png + i * 4;
            fourBytes[i] = Premultiply(p[0], p[1], p[2], p[3]);
        }
#endif
    }

    // OpenGL用のテクスチャを生成する
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, png);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 解放処理
    stbi_image_free(png);
    LWallpaperPal::ReleaseBytes(address);

    LWallpaperTextureManager::TextureInfo* textureInfo = new LWallpaperTextureManager::TextureInfo();
    if (textureInfo != NULL)
    {
        textureInfo->fileName = fileName;
        textureInfo->width = width;
        textureInfo->height = height;
        textureInfo->id = textureId;

        _textures.PushBack(textureInfo);
    }

    return textureInfo;

}

void LWallpaperTextureManager::ReleaseTextures()
{
    for (Csm::csmUint32 i = 0; i < _textures.GetSize(); i++)
    {
        delete _textures[i];
    }

    _textures.Clear();
}

void LWallpaperTextureManager::ReleaseTexture(Csm::csmUint32 textureId)
{
    for (Csm::csmUint32 i = 0; i < _textures.GetSize(); i++)
    {
        if (_textures[i]->id != textureId)
        {
            continue;
        }
        delete _textures[i];
        _textures.Remove(i);
        break;
    }
}

void LWallpaperTextureManager::ReleaseTexture(std::string fileName)
{
    for (Csm::csmUint32 i = 0; i < _textures.GetSize(); i++)
    {
        if (_textures[i]->fileName == fileName)
        {
            delete _textures[i];
            _textures.Remove(i);
            break;
        }
    }
}
