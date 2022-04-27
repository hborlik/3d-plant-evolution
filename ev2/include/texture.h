/**
 * @file texture.h
 * @brief 
 * @date 2022-04-26
 * 
 */
#ifndef EV2_TEXTURE_H
#define EV2_TESTURE_H

#include <string>

#include <ev_gl.h>

namespace ev2 {

class Texture {
public:
    explicit Texture(gl::TextureType texture_type);

    ~Texture() {
        if (handle != -1)
            glDeleteTextures(1, &handle);
    }

    Texture(Texture&& o) : texture_type{o.texture_type} {
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

    void generate_mips();

    /**
     * @brief Get the Handle to the texture
     * 
     * @return GLuint 
     */
    GLuint get_handle() const noexcept {return handle;}

    void bind() const {glBindTexture((GLenum)texture_type, handle);}
    void unbind() const {glBindTexture((GLenum)target, 0);}

    gl::TextureType type() const noexcept {return texture_type;}

    /**
     * @brief Set the image data for a 2D image, texture must have the TextureType::TEXTURE_2D type
     * 
     * @param data 
     * @param dataFormat 
     * @param dataType 
     * @param internalFormat 
     * @param width 
     * @param height 
     */
    void set_data2D(const unsigned char* data, gl::PixelFormat dataFormat, gl::PixelType dataType, gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height);

    void set_data3D(const unsigned char* data, gl::PixelFormat dataFormat, gl::PixelType dataType, gl::TextureInternalFormat internalFormat, uint32_t width, uint32_t height, gl::TextureTarget side);

protected:
    const gl::TextureType texture_type;
    GLuint handle = 0;
};

}

#endif // EV2_TEXTURE_H