/**
 * @file buffer.h
 * @author Hunter Borlik
 * @brief opengl buffer
 * @version 0.2
 * @date 2019-09-21
 * 
 * 
 */
#ifndef EV2_BUFFER_H
#define EV2_BUFFER_H

#include <vector>

#include <ev_gl.h>

namespace ev2 {

class Buffer {
public:

    Buffer(gl::BindingTarget target, gl::Usage usage);

    template<typename T>
    Buffer(gl::BindingTarget target, gl::Usage usage, const std::vector<T>& data) : target{target}, usage{usage} {
        glGenBuffers(1, &gl_reference);
        CopyData(data);
    }

    virtual ~Buffer();

    Buffer(const Buffer& o) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& o) : target{o.target}, usage{o.usage} {
        std::swap(gl_reference, o.gl_reference);
    }

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

    GLuint handle() const noexcept { return gl_reference; }

    /**
     * @brief Get the size in bytes of buffer
     * 
     * @return size_t 
     */
    size_t size() const noexcept { return buf_size; }


    /**
     * @brief intended binding target of this buffer
     */
    const gl::BindingTarget target;

    /**
     * @brief intended usage type
     */
    const gl::Usage usage;

private:

    /**
     * @brief size in bytes of copied data 
     */
    size_t buf_size;

    GLuint gl_reference = -1;
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

/**
 * @brief single element update in buffer data, offset and size must define a range lying entirely within the buffer object's data store
 * 
 * @tparam T 
 * @param source 
 * @param offset 
 */
template<typename T>//, typename>
void Buffer::SubData(const T& source, uint32_t offset) {
    glBindBuffer((GLenum)target, gl_reference);
    GL_CHECKED_CALL(glBufferSubData((GLenum)target, offset, sizeof(T), &source));
    glBindBuffer((GLenum)target, 0);
}

/**
 * @brief update array, offset and size must define a range lying entirely within the buffer object's data store
 * 
 * @tparam T 
 * @param source 
 * @param offset 
 * @param stride 
 */
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

} // ev2

#endif // EV2_BUFFER_H