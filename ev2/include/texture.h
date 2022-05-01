/**
 * @file texture.h
 * @brief 
 * @date 2022-04-26
 * 
 */
#ifndef EV2_TEXTURE_H
#define EV2_TESTURE_H

#include <string>
#include <unordered_map>
#include <memory>

#include <ev_gl.h>

namespace ev2 {

class Texture {
public:
    explicit Texture(gl::TextureType texture_type);
    Texture(gl::TextureType texture_type, gl::TextureFilterMode filterMode);

    ~Texture() {
        if (handle != 0)
            glDeleteTextures(1, &handle);
    }

    Texture(Texture &&o) : texture_type{o.texture_type},
                           internal_format{o.internal_format},
                           pixel_format{o.pixel_format},
                           pixel_type{o.pixel_type}
    {
        std::swap(handle, o.handle);
    }

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&) = delete;

    /**
     * @brief Set the Texture wrapping behavior
     * 
     * @param wrap edge
     * @param mode behavior
     */
    void set_wrap_mode(gl::TextureParamWrap wrap, gl::TextureWrapMode mode);

    /**
     * @brief Set the Filter Mode udes when determining pixle color
     * 
     * @param filter filter function
     * @param mode behavior 
     */
    void set_filter_mode(gl::TextureParamFilter filter, gl::TextureFilterMode mode);

    /**
     * @brief generate the mip maps for this texture
     * 
     */
    void generate_mips();

    /**
     * @brief Get the Handle to the texture
     * 
     * @return GLuint 
     */
    GLuint get_handle() const noexcept {return handle;}

    void bind() const {glBindTexture((GLenum)texture_type, handle);}
    void unbind() const {glBindTexture((GLenum)texture_type, 0);}

    gl::TextureType type() const noexcept {return texture_type;}
    gl::TextureInternalFormat get_internal_format() const noexcept {return internal_format;}
    gl::PixelFormat get_pixel_format() const noexcept {return pixel_format;}
    gl::PixelType get_pixel_type() const noexcept {return pixel_type;}

    /**
     * @brief Allocate and set the image data for a 2D image, texture must have the TextureType::TEXTURE_2D type
     * 
     * @param data nullptr to 0 fill texture memory
     * @param dataFormat 
     * @param dataType 
     * @param internalFormat 
     * @param width 
     * @param height 
     */
    void set_data2D(gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height, gl::PixelFormat dataFormat, gl::PixelType dataType, const unsigned char* data);

    void set_data3D(gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height, gl::PixelFormat dataFormat, gl::PixelType dataType, const unsigned char* data, gl::TextureTarget side);

protected:
    const gl::TextureType texture_type;
    gl::TextureInternalFormat internal_format;
    gl::PixelFormat pixel_format;
    gl::PixelType pixel_type;
    GLuint handle = 0;
};

class RenderBuffer {
public:
    RenderBuffer() {
        glGenRenderbuffers(1, &gl_reference);
    }

    ~RenderBuffer() {
        if (gl_reference != 0)
            glDeleteRenderbuffers(1, &gl_reference);
    }

    RenderBuffer(RenderBuffer&& o) {
        std::swap(gl_reference, o.gl_reference);
    }

    RenderBuffer(const RenderBuffer& o) = delete;
    RenderBuffer& operator              = (const RenderBuffer&) = delete;
    RenderBuffer& operator              = (RenderBuffer&&)      = delete;

    void bind() {glBindRenderbuffer(GL_RENDERBUFFER, gl_reference);}
    void unbind() {glBindRenderbuffer(GL_RENDERBUFFER, 0);}

    void set_data(gl::RenderBufferInternalFormat format, uint32_t width, uint32_t height) {
        bind();
        glRenderbufferStorage(GL_RENDERBUFFER, (GLenum)format, width, height);
        unbind();
        this->format = format;
    }

    GLuint get_handle() const noexcept {return gl_reference;}

    gl::RenderBufferInternalFormat get_format() const noexcept {return format;}

private:
    GLuint gl_reference = 0;
    gl::RenderBufferInternalFormat format;
};

/**
 * @brief Frame Buffer Object
 * 
 */
class FBO {
public:
    FBO(gl::FBOTarget target) : target{target} {
        GL_CHECKED_CALL(glGenFramebuffers(1, &gl_reference));
    }

    ~FBO() {
        if (gl_reference != 0)
            glDeleteFramebuffers(1, &gl_reference);
    }

    FBO(FBO&& o) {
        std::swap(gl_reference, o.gl_reference);
        std::swap(target, o.target);
    }

    FBO(const FBO& o) = delete;
    FBO& operator=(const FBO&) = delete;
    FBO& operator=(FBO&&) = delete;

    void resize_all(uint32_t width, uint32_t height);

    /**
     * @brief Set the bind target. Do not change while framebuffer is bound
     * 
     * @param target 
     */
    void set_bind_target(gl::FBOTarget target) {this->target = target;}

    void bind() const {glBindFramebuffer((GLenum)target, gl_reference);}
    void unbind() const {glBindFramebuffer((GLenum)target, 0);}

    /**
     * @brief check that this framebuffer has all the required attachments for its specified binding target
     * 
     * @return true it is ready
     * @return false it is not ready
     */
    bool check();

    /**
     * @brief attach a texture to the framebuffer. currently only supports 2d textures
     * 
     * @param texture 
     * @return true 
     * @return false 
     */
    bool attach(std::shared_ptr<Texture> texture, gl::FBOAttachment attachment_point);

    bool attach_renderbuffer(gl::RenderBufferInternalFormat format, uint32_t width, uint32_t height, gl::FBOAttachment attachment_point);

    void detach(gl::FBOAttachment attachment_point);

private:
    gl::FBOTarget target;
    GLuint gl_reference = 0;

    std::unordered_map<gl::FBOAttachment, std::shared_ptr<Texture>> attachments;
    std::unordered_map<gl::FBOAttachment, RenderBuffer> rb_attachments;
};

}

#endif // EV2_TEXTURE_H