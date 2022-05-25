#include <texture.h>

#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace ev2 {

Texture::Texture(gl::TextureType texture_type) : texture_type{texture_type} {
    GL_CHECKED_CALL(glGenTextures(1, &handle));
    
    set_filter_mode(gl::TextureParamFilter::TEXTURE_MIN_FILTER, gl::TextureFilterMode::LINEAR);
    set_filter_mode(gl::TextureParamFilter::TEXTURE_MAG_FILTER, gl::TextureFilterMode::LINEAR);

    set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_S, gl::TextureWrapMode::REPEAT);
    set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_T, gl::TextureWrapMode::REPEAT);
}

Texture::Texture(gl::TextureType texture_type, gl::TextureFilterMode filterMode) : texture_type{texture_type} {
    GL_CHECKED_CALL(glGenTextures(1, &handle));

    set_filter_mode(gl::TextureParamFilter::TEXTURE_MIN_FILTER, filterMode);
    set_filter_mode(gl::TextureParamFilter::TEXTURE_MAG_FILTER, filterMode);

    set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_S, gl::TextureWrapMode::REPEAT);
    set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_T, gl::TextureWrapMode::REPEAT);
}

void Texture::set_wrap_mode(gl::TextureParamWrap wrap, gl::TextureWrapMode mode) {
    glBindTexture((GLenum)texture_type, handle);
    GL_CHECKED_CALL(glTexParameteri((GLenum)texture_type, (GLenum)wrap, (GLenum)mode));
    glBindTexture((GLenum)texture_type, 0);
}

void Texture::set_border_color(const glm::vec4& color) {
    glBindTexture((GLenum)texture_type, handle);
    GL_CHECKED_CALL(glTexParameterfv((GLenum)texture_type, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color)));
    glBindTexture((GLenum)texture_type, 0);
}

void Texture::set_filter_mode(gl::TextureParamFilter filter, gl::TextureFilterMode mode) {
    glBindTexture((GLenum)texture_type, handle);
    GL_CHECKED_CALL(glTexParameteri((GLenum)texture_type, (GLenum)filter, (GLenum)mode));
    glBindTexture((GLenum)texture_type, 0);
}

void Texture::generate_mips() {
    bind();
    glGenerateMipmap((GLenum)texture_type);
    unbind();
}

void Texture::set_data2D(gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height, gl::PixelFormat dataFormat, gl::PixelType dataType, const unsigned char* data) {
    bind();
    GL_CHECKED_CALL(glTexImage2D((GLenum)gl::TextureTarget::TEXTURE_2D, 0, (GLint)internalFormat, width, height, 0, (GLenum)dataFormat, (GLenum)dataType, data));
    unbind();
    internal_format = internalFormat;
    pixel_format = dataFormat;
    pixel_type = dataType;
}

void Texture::set_data3D(gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height, gl::PixelFormat dataFormat, gl::PixelType dataType, const unsigned char* data, gl::TextureTarget side) {
    bind();
    GL_CHECKED_CALL(glTexImage2D((GLenum)side, 0, (GLint)internalFormat, width, height, 0, (GLenum)dataFormat, (GLenum)dataType, data));
    unbind();
    internal_format = internalFormat;
    pixel_format = dataFormat;
    pixel_type = dataType;
}

// FBO

void FBO::resize_all(uint32_t width, uint32_t height) {
    for (auto& tex_at : attachments) {
        auto &tex = tex_at.second.texture;
        tex->set_data2D(tex->get_internal_format(), width, height, tex->get_pixel_format(), tex->get_pixel_type(), nullptr);
    }
    for (auto& rb_at : rb_attachments) {
        auto &rb = rb_at.second;
        rb.set_data(rb.get_format(), width, height);
    }
}

bool FBO::check() {
    bind();
    // enable all drawbuffers
    int max_binding = 0;
    for (auto &a : attachments) {
        if (a.second.location >= 0) 
        max_binding = max_binding > a.second.location ? max_binding : a.second.location;
    }
    std::vector<GLenum> dbtgt(max_binding + 1);
    int i = 0;
    for (auto &a : attachments) {
        if (a.second.location >= 0) 
        dbtgt[a.second.location] = (GLenum)a.first;
    }

    GL_CHECKED_CALL(glDrawBuffers(dbtgt.size(), dbtgt.data()));

    GLenum err = glCheckFramebufferStatus((GLenum)target);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
        // TODO error println
    }
    unbind();
    return err == GL_FRAMEBUFFER_COMPLETE;
}

bool FBO::attach(std::shared_ptr<Texture> texture, gl::FBOAttachment attachment_point, int location) {
    if ((attachment_point == gl::FBOAttachment::DEPTH) || (attachment_point == gl::FBOAttachment::DEPTH_STENCIL) || (attachment_point == gl::FBOAttachment::STENCIL)) 
    {
        assert(location == -1);
    }
    if (attachments.find(attachment_point) != attachments.end())
        return false;
    if (texture && texture->type() == gl::TextureType::TEXTURE_2D) {
        clearGLErrors();
        // glNamedFramebufferTexture(gl_reference, (GLenum)attachment_point, texture->get_handle(), 0);
        bind();
        glFramebufferTexture2D((GLenum)target, (GLenum)attachment_point, (GLenum)texture->type(), texture->get_handle(), 0);
        unbind();
        if (!isGLError()) {
            attachments.insert(std::pair{attachment_point, AttachmentBinding{location, texture}});
            rb_attachments.erase(attachment_point);

            return true;
        }
    }
    std::cout << "Failed to attach FBO attachment" << std::endl;
    return false;
}

bool FBO::attach_renderbuffer(gl::RenderBufferInternalFormat format, uint32_t width, uint32_t height, gl::FBOAttachment attachment_point) {
    // create new renderbuffer
    auto ip = rb_attachments.emplace(std::pair{attachment_point, RenderBuffer{}});
    if (!ip.second)
        return false;

    RenderBuffer &r_buffer = ip.first->second;
    r_buffer.set_data(format, width, height);

    // attempt to attach the renderbuffer
    clearGLErrors();
    // glNamedFramebufferRenderbuffer(gl_reference, (GLenum)attachment_point, GL_RENDERBUFFER, r_buffer.get_handle());
    bind();
    glFramebufferRenderbuffer((GLenum)target, (GLenum)attachment_point, GL_RENDERBUFFER, r_buffer.get_handle());
    unbind();
    if (!isGLError()) {
        attachments.erase(attachment_point);

        return true;
    }
    return false;
}

}