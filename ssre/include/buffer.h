/**
 * @file buffer.h
 * @author Hunter Borlik 
 * @brief opengl buffer
 * @version 0.1
 * @date 2019-09-21
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_BUFFER_H
#define SSRE_BUFFER_H

#include <vector>

#include <ssre_gl.h>

namespace ssre {

class Buffer {
public:

    Buffer(gl::BindingTarget target, gl::Usage usage);
    virtual ~Buffer();

    Buffer(const Buffer& o) = delete;
    Buffer(Buffer&& o) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&) = delete;

    /**
     * @brief Fill entire buffer with data
     * 
     * @tparam T 
     * @param source 
     */
    template<typename T>
    void CopyData(const std::vector<T>& source);

    /**
     * @brief Update part of data in buffer. Buffer should have data allocated before call is made to sub data
     * 
     * @tparam T 
     * @param source 
     * @param offset 
     */
    template<typename T>//, typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
    void SubData(const T& source, uint32_t offset);

    /**
     * @brief Update part of data in buffer. Buffer should have data allocated before call is made to sub data
     * 
     * @tparam T 
     * @param source 
     * @param offset 
     */
    template<typename T>
    void SubData(const std::vector<T>& source, uint32_t offset, uint32_t stride);

    /**
     * @brief Allocate buffer data
     * 
     * @param bytes number of bytes to allocate
     */
    void Allocate(std::size_t bytes);

    /**
     * @brief Bind this buffer to its target
     */
    void Bind() { glBindBuffer((GLenum)target, gl_reference); }

    /**
     * @brief Unbind this buffer from its target
     */
    void Unbind() {glBindBuffer((GLenum)target, 0);}

    GLuint Handle() noexcept { return gl_reference; }

    /**
     * @brief intended binding target of this buffer
     */
    const gl::BindingTarget target;

    /**
     * @brief intended usage type
     */
    const gl::Usage usage;

    /**
     * @brief Get the size in bytes of buffer
     * 
     * @return size_t 
     */
    size_t size() const noexcept { return buf_size; }

private:

    /**
     * @brief size in bytes of copied data 
     */
    size_t buf_size;

    GLuint gl_reference;
};

template<typename T>
void Buffer::CopyData(const std::vector<T>& source) {
    if(!source.empty()) {
        glBindBuffer((GLenum)target, gl_reference);
        glBufferData((GLenum)target, sizeof(T) * source.size(), source.data(), (GLenum)usage);
        glBindBuffer((GLenum)target, 0);
        buf_size = sizeof(T) * source.size();
    }
}

template<typename T>//, typename>
void Buffer::SubData(const T& source, uint32_t offset) {
    glBindBuffer((GLenum)target, gl_reference);
    GL_CHECKED_CALL(glBufferSubData((GLenum)target, offset, sizeof(T), &source));
    glBindBuffer((GLenum)target, 0);
}

template<typename T>
void Buffer::SubData(const std::vector<T>& source, uint32_t offset, uint32_t stride) {
    if(!source.empty()) {
        glBindBuffer((GLenum)target, gl_reference);
        for(size_t i = 0; i < source.size(); i++) {
            GL_CHECKED_CALL(glBufferSubData((GLenum)target, offset + i * stride, sizeof(T), &source[i]));
        }
        glBindBuffer((GLenum)target, 0);
    }
}

} // ssre

#endif // SSRE_BUFFER_H