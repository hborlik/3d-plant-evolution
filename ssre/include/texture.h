/**
 * @file texture.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-04
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_TEXTURE_H
#define SSRE_TEXTURE_H

#include <string>

#include <ssre_gl.h>

namespace ssre {

class Texture {
public:
    Texture(gl::TextureTarget target, std::string name);
    Texture(const Texture&) = delete;
    virtual ~Texture();

    Texture& operator=(const Texture&) = delete;

    /**
     * @brief Set the Texture wrapping behavior
     * 
     * @param wrap edge
     * @param mode behavior
     */
    void setWrapMode(gl::TextureParamWrap wrap, gl::TextureWrapMode mode);

    /**
     * @brief Set the Filter Mode udes when determining pixle color
     * 
     * @param filter filter function
     * @param mode behavior 
     */
    void setFilterMode(gl::TextureParamFilter filter, gl::TextureFilterMode mode);

    void generateMipMap();

    /**
     * @brief Get the Handle to the texture
     * 
     * @return GLuint 
     */
    GLuint getHandle() const noexcept {return handle;}

    void bind() const {glBindTexture((GLenum)target, handle);}

    gl::TextureTarget type() const noexcept {return target;}

    const std::string& getName() const noexcept {return name;}

protected:
    const gl::TextureTarget target;
    const std::string name;
    GLuint handle = 0;
};

class Texture2D : public Texture {
public:
    Texture2D(std::string name);
    virtual ~Texture2D();

    Texture2D& operator=(const Texture2D&) = delete;

    /**
     * @brief Set the 2D Data for this texture, this uploads direct to GPU.
     * 
     * @param data 
     */
    void setData(const unsigned char* data, gl::PixelFormat dataFormat, gl::PixelType dataType, gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height);
};

class Texture3D : public Texture {
public:
    Texture3D(std::string name);
    virtual ~Texture3D();

    Texture3D& operator=(const Texture3D&) = delete;

    void setData(const unsigned char* data, gl::PixelFormat dataFormat, gl::PixelType dataType, gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height, uint32_t side);
};

}

#endif // SSRE_TEXTURE_H