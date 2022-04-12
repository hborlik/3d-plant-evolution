/**
 * @file texture.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-05
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <texture.h>

using namespace ssre;

Texture::Texture(gl::TextureTarget target, std::string name) : target{target}, name{std::move(name)} {
    glGenTextures(1, &handle);
    setWrapMode(gl::TextureParamWrap::TEXTURE_WRAP_S, gl::TextureWrapMode::REPEAT);
    setWrapMode(gl::TextureParamWrap::TEXTURE_WRAP_T, gl::TextureWrapMode::REPEAT);
    setFilterMode(gl::TextureParamFilter::TEXTURE_MIN_FILTER, gl::TextureFilterMode::LINEAR);
    setFilterMode(gl::TextureParamFilter::TEXTURE_MAG_FILTER, gl::TextureFilterMode::LINEAR);
}

Texture::~Texture() {
    glDeleteTextures(1, &handle);
}

void Texture::setWrapMode(gl::TextureParamWrap wrap, gl::TextureWrapMode mode) {
    glBindTexture((GLenum)target, handle);
    glTexParameteri((GLenum)target, (GLenum)wrap, (GLenum)mode);
    glBindTexture((GLenum)target, 0);
}

void Texture::setFilterMode(gl::TextureParamFilter filter, gl::TextureFilterMode mode) {
    glBindTexture((GLenum)target, handle);
    glTexParameteri((GLenum)target, (GLenum)filter, (GLenum)mode);
    glBindTexture((GLenum)target, 0);
}

void Texture::generateMipMap() {
    bind();
    glGenerateMipmap((GLenum)target);
    glBindTexture((GLenum)target, 0);
}

///////////////////////////////////

Texture2D::Texture2D(std::string name) : Texture{gl::TextureTarget::TEXTURE_2D, std::move(name)} {

}

Texture2D::~Texture2D() {

}

void Texture2D::setData(const unsigned char* data, gl::PixelFormat dataFormat, gl::PixelType dataType, gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height) {
    bind();
    glTexImage2D((GLenum)target, 0, (GLint)internalFormat, width, height, 0, (GLenum)dataFormat, (GLenum)dataType, data);
    glBindTexture((GLenum)target, 0);
}

///////////////////////////////////////////

Texture3D::Texture3D(std::string name) : Texture{gl::TextureTarget::TEXTURE_CUBE_MAP, std::move(name)} {

}

Texture3D::~Texture3D() {

}

void Texture3D::setData(const unsigned char* data, gl::PixelFormat dataFormat, gl::PixelType dataType, gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height, uint32_t side) {
    bind();
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, (GLint)internalFormat, width, height, 0, (GLenum)dataFormat, (GLenum)dataType, data);
    glBindTexture((GLenum)target, 0);
}