/**
 * @file buffer.cpp
 * @author Hunter Borlik
 * @brief OpenGL buffer 
 * @version 0.2
 * @date 2019-09-21
 * 
 * 
 */

#include <buffer.h>

using namespace ev2;
using namespace ev2::gl;

Buffer::Buffer(BindingTarget target, Usage usage) : target{target}, usage{usage}, buf_size{}, gl_reference{} {
    glGenBuffers(1, &gl_reference);
}

Buffer::~Buffer() {
    glDeleteBuffers(1, &gl_reference);
}

void Buffer::Allocate(std::size_t bytes) {
    glBindBuffer((GLenum)target, gl_reference);
    GL_CHECKED_CALL(glBufferData((GLenum)target, bytes, NULL, (GLenum)usage));
    glBindBuffer((GLenum)target, 0);
    buf_size = bytes;
}