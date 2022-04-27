#include <texture.h>

namespace ev2 {

Texture::Texture(gl::TextureType texture_type) : texture_type{texture_type} {
    glGenTextures(1, &handle);
    set_filter_mode(gl::TextureParamFilter::TEXTURE_MIN_FILTER, gl::TextureFilterMode::LINEAR);
    set_filter_mode(gl::TextureParamFilter::TEXTURE_MAG_FILTER, gl::TextureFilterMode::LINEAR);
}

void Texture::set_wrap_mode(gl::TextureParamWrap wrap, gl::TextureWrapMode mode) {
    glBindTexture((GLenum)texture_type, handle);
    glTexParameteri((GLenum)texture_type, (GLenum)wrap, (GLenum)mode);
    glBindTexture((GLenum)texture_type, 0);
}

void Texture::set_filter_mode(gl::TextureParamFilter filter, gl::TextureFilterMode mode) {
    glBindTexture((GLenum)texture_type, handle);
    glTexParameteri((GLenum)texture_type, (GLenum)filter, (GLenum)mode);
    glBindTexture((GLenum)texture_type, 0);
}

void Texture::generate_mips() {
    bind();
    glGenerateMipmap((GLenum)texture_type);
    unbind();
}

void Texture::set_data2D(const unsigned char* data, gl::PixelFormat dataFormat, gl::PixelType dataType, gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height) {
    bind();
    glTexImage2D((GLenum)gl::TextureTarget::TEXTURE_2D, 0, (GLint)internalFormat, width, height, 0, (GLenum)dataFormat, (GLenum)dataType, data);
    unbind();
    internal_format = internalFormat;
}

void Texture::set_data3D(const unsigned char* data, gl::PixelFormat dataFormat, gl::PixelType dataType, gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height, gl::TextureTarget side) {
    bind();
    glTexImage2D((GLenum)side, 0, (GLint)internalFormat, width, height, 0, (GLenum)dataFormat, (GLenum)dataType, data);
    unbind();
    internal_format = internalFormat;
}

// FBO

void FBO::resize_all(uint32_t width, uint32_t height) {
    for (auto& tex_at : attachments) {
        auto &tex = tex_at.second;
        tex->set_data2D(nullptr, gl::PixelFormat::RGB, tex->get_pixel_type(), tex->get_internal_format(), width, height);
    }
}

bool FBO::check() {
    GLenum err = glCheckNamedFramebufferStatus(gl_reference, (GLenum)target);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
        // TODO error println
    }
    return err == GL_FRAMEBUFFER_COMPLETE;
}

bool FBO::attach(std::shared_ptr<Texture> texture, gl::FBOAttachment attachment_point) {
    if (texture && texture->type() == gl::TextureType::TEXTURE_2D) {
        clearGLErrors();
        glFramebufferTexture2D((GLenum)target, (GLenum)attachment_point, (GLenum)texture->type(), texture->get_handle(), 0);
        if (!isGLError()) {
            attachments.insert_or_assign(attachment_point, texture);
            return true;
        }
    }
    return false;
}

}