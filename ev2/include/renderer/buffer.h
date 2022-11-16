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

#include <renderer/ev_gl.h>

namespace ev2::renderer {

class Buffer {
public:

    Buffer(gl::BindingTarget target, gl::Usage usage);

    template<typename T>
    Buffer(gl::BindingTarget target, gl::Usage usage, const std::vector<T>& data) : target{target}, usage{usage} {
        glGenBuffers(1, &gl_reference);
        copy_data(data);
    }

    Buffer(gl::BindingTarget target, gl::Usage usage, std::size_t size, const void* data) : target{target}, usage{usage} {
        glGenBuffers(1, &gl_reference);
        copy_data(size, data);
    }

    virtual ~Buffer();

    Buffer(const Buffer& o) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& o) {
        *this = std::move(o);
    }

    Buffer& operator=(Buffer&& o) {
        std::swap(gl_reference, o.gl_reference);
        std::swap(target, o.target);
        std::swap(usage, o.usage);

        return *this;
    }

    /**
     * @brief Allocate buffer large enough to contain all data in source and copy data into buffer.
     * 
     * @tparam T 
     * @param source 
     */
    template<typename T>
    void copy_data(const std::vector<T>& source);

    void copy_data(std::size_t size, const void* data);

    /**
     * @brief Update part of data in buffer. Buffer should have data allocated before call is made to sub data
     * 
     * @tparam T 
     * @param source 
     * @param offset 
     */
    template<typename T>//, typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
    void sub_data(const T& source, uint32_t offset);

    /**
     * @brief Update part of data in buffer. Buffer should have data allocated before call is made to sub data
     * 
     * @tparam T 
     * @param source 
     * @param offset 
     */
    template<typename T>
    void sub_data(const std::vector<T>& source, uint32_t offset, uint32_t stride);

    /**
     * @brief Allocate buffer data
     * 
     * @param bytes number of bytes to allocate
     */
    void allocate(std::size_t bytes);

    /**
     * @brief Bind this buffer to its target
     */
    void bind() const { glBindBuffer((GLenum)target, gl_reference); }

    /**
     * @brief Bind this buffer at given index of the binding point.
     * Buffer target must be one of GL_ATOMIC_COUNTER_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER or GL_SHADER_STORAGE_BUFFER.
     */
    void bind(GLuint index) {glBindBufferBase((GLenum)target, index, gl_reference);}

    /**
     * @brief binds the range to the generic buffer binding point specified by target and to given index of the binding point.
     * Buffer target must be one of GL_ATOMIC_COUNTER_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER or GL_SHADER_STORAGE_BUFFER.
     * 
     * @param index index of the binding point 
     * @param size  amount of data that should be available to read (from the offset) while the buffer is bound to this index
     * @param offset offset into buffer
     */
    void bind_range(GLuint index, GLuint size, GLint offset = 0) {
        // offset can be positive or negative
        int buf_offset = ((offset % capacity) + capacity) % capacity;
        assert(buf_offset + size < capacity);
        glBindBufferRange((GLenum)target, index, gl_reference, buf_offset, size);
    }

    /**
     * @brief Unbind this buffer from its target
     */
    void unbind() const {glBindBuffer((GLenum)target, 0);}

    GLuint handle() const noexcept { return gl_reference; }

    /**
     * @brief Get the size in bytes of buffer
     * 
     * @return size_t 
     */
    size_t get_capacity() const noexcept { return capacity; }

    gl::BindingTarget get_binding_target() const noexcept {return target;}

    gl::Usage get_usage() const noexcept {return usage;}

private:

    /**
     * @brief size in bytes of copied data 
     */
    size_t capacity = 0;

    GLuint gl_reference = -1;

    /**
     * @brief binding target of this buffer
     */
    gl::BindingTarget target;

    /**
     * @brief usage type of buffer
     */
    gl::Usage usage;
};

template<typename T>
void Buffer::copy_data(const std::vector<T>& source) {
    if(!source.empty()) {
        glBindBuffer((GLenum)target, gl_reference);
        glBufferData((GLenum)target, sizeof(T) * source.size(), source.data(), (GLenum)usage);
        glBindBuffer((GLenum)target, 0);
        capacity = sizeof(T) * source.size();
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
void Buffer::sub_data(const T& source, uint32_t offset) {
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
void Buffer::sub_data(const std::vector<T>& source, uint32_t offset, uint32_t stride) {
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