/**
 * @file ssre_gl.h
 * @brief gl includes for ssre
 * @version 0.2
 * @date 2019-09-09
 * 
 * 
 */
#ifndef EV_GL_H
#define EV_GL_H

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#define GL_CHECKED_CALL(call) {isGLError(); (call); if(isGLError()) std::cerr << __FILE__ << ":" << __LINE__ << " GL_CHECKED_CALL error" << std::endl;}

#ifndef NDEBUG
#define GL_ERROR_CHECK(call, var) {isGLError(); (call); var = isGLError();}
#else
#define GL_ERROR_CHECK(call, var) {(call);}
#endif

namespace ev2 {
    /**
     * @brief check for an error since last isGLError call
     * 
     * @return true error was detected
     * @return false no error
     */
    bool isGLError();

    namespace gl {
    
    enum class DataType : GLenum {
        UNKNOWN             = GL_FALSE,
        BYTE                = GL_BYTE,
        UNSIGNED_BYTE       = GL_UNSIGNED_BYTE,
        SHORT               = GL_SHORT,
        UNSIGNED_SHORT      = GL_UNSIGNED_SHORT,
        INT                 = GL_INT,
        UNSIGNED_INT        = GL_UNSIGNED_INT,
        FIXED               = GL_FIXED,
        HALF_FLOAT          = GL_HALF_FLOAT,
        FLOAT               = GL_FLOAT,
        DOUBLE              = GL_DOUBLE
    };

    enum class BindingTarget : GLenum {
        ARRAY               = GL_ARRAY_BUFFER,
        ATOMIC_COUNTER      = GL_ATOMIC_COUNTER_BUFFER,
        COPY_READ           = GL_COPY_READ_BUFFER,
        COPY_WRITE          = GL_COPY_WRITE_BUFFER,
        DISPATCH_INDIRECT   = GL_DISPATCH_INDIRECT_BUFFER,
        DRAW_INDIRECT       = GL_DRAW_INDIRECT_BUFFER,
        ELEMENT_ARRAY       = GL_ELEMENT_ARRAY_BUFFER,
        PIXEL_PACK          = GL_PIXEL_PACK_BUFFER,
        PIXEL_UNPACK        = GL_PIXEL_UNPACK_BUFFER,
        QUERY               = GL_QUERY_BUFFER,
        SHADER_STORAGE      = GL_SHADER_STORAGE_BUFFER,
        TEXTURE             = GL_TEXTURE_BUFFER,
        TRANSFORM_FEEDBACK  = GL_TRANSFORM_FEEDBACK_BUFFER,
        UNIFORM             = GL_UNIFORM_BUFFER
    };

    enum class Usage : GLenum {
        STREAM_DRAW         = GL_STREAM_DRAW,
        STREAM_READ         = GL_STREAM_READ,
        STREAM_COPY         = GL_STREAM_COPY,
        STATIC_DRAW         = GL_STATIC_DRAW,
        STATIC_READ         = GL_STATIC_READ,
        STATIC_COPY         = GL_STATIC_COPY,
        DYNAMIC_DRAW        = GL_DYNAMIC_DRAW,
        DYNAMIC_READ        = GL_DYNAMIC_READ,
        DYNAMIC_COPY        = GL_DYNAMIC_COPY
    };

    enum class GLSLShaderType : GLenum {
        VERTEX_SHADER           = GL_VERTEX_SHADER,
        FRAGMENT_SHADER         = GL_FRAGMENT_SHADER,
        GEOMETRY_SHADER         = GL_GEOMETRY_SHADER,
        TESS_EVALUATION_SHADER  = GL_TESS_EVALUATION_SHADER, 
        TESS_CONTROL_SHADER     = GL_TESS_CONTROL_SHADER
    };


    enum class TextureParamWrap : GLenum {
        TEXTURE_WRAP_S = GL_TEXTURE_WRAP_S,
        TEXTURE_WRAP_T = GL_TEXTURE_WRAP_T,
        TEXTURE_WRAP_R = GL_TEXTURE_WRAP_R
    };

    enum class TextureWrapMode : GLenum {
        REPEAT = GL_REPEAT,
        MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
        CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
        CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
    };

    enum class TextureParamFilter : GLenum {
        TEXTURE_MIN_FILTER = GL_TEXTURE_MIN_FILTER,
        TEXTURE_MAG_FILTER = GL_TEXTURE_MAG_FILTER
    };

    enum class TextureFilterMode : GLenum {
        NEAREST = GL_NEAREST,
        LINEAR = GL_LINEAR,
        NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
    };

    enum class TextureTarget : GLenum {
        TEXTURE_2D = GL_TEXTURE_2D,
        TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP
    };

    enum class TextureInternalFormat : GLenum { // internal storage format
        RED = GL_RED,
        RG = GL_RG,
        RGB = GL_RGB,
        BGR = GL_BGR,
        RGBA = GL_RGBA,
        BGRA = GL_BGRA,
        DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
        DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,
        DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,
        DEPTH_COMPONENT32 = GL_DEPTH_COMPONENT32,
        DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,
        DEPTH_STENCIL = GL_DEPTH_STENCIL
    };

    enum class PixelFormat : GLenum { // input format
        RED = GL_RED,
        RG = GL_RG,
        RGB = GL_RGB,
        BGR = GL_BGR,
        RGBA = GL_RGBA,
        BGRA = GL_BGRA,
        DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
        DEPTH_STENCIL = GL_DEPTH_STENCIL
    };

    enum class PixelType : GLenum {
        UNSIGNED_BYTE = GL_UNSIGNED_BYTE, 
        BYTE = GL_BYTE, 
        UNSIGNED_SHORT = GL_UNSIGNED_SHORT, 
        SHORT = GL_SHORT, 
        UNSIGNED_INT = GL_UNSIGNED_INT, 
        INT = GL_INT, 
        HALF_FLOAT = GL_HALF_FLOAT, 
        FLOAT = GL_FLOAT, 
        UNSIGNED_BYTE_3_3_2 = GL_UNSIGNED_BYTE_3_3_2, 
        UNSIGNED_BYTE_2_3_3_REV = GL_UNSIGNED_BYTE_2_3_3_REV, 
        UNSIGNED_SHORT_5_6_5 = GL_UNSIGNED_SHORT_5_6_5, 
        UNSIGNED_SHORT_5_6_5_REV = GL_UNSIGNED_SHORT_5_6_5_REV, 
        UNSIGNED_SHORT_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4, 
        UNSIGNED_SHORT_4_4_4_4_REV = GL_UNSIGNED_SHORT_4_4_4_4_REV, 
        UNSIGNED_SHORT_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1, 
        UNSIGNED_SHORT_1_5_5_5_REV = GL_UNSIGNED_SHORT_1_5_5_5_REV, 
        UNSIGNED_INT_8_8_8_8 = GL_UNSIGNED_INT_8_8_8_8, 
        UNSIGNED_INT_8_8_8_8_REV = GL_UNSIGNED_INT_8_8_8_8_REV, 
        UNSIGNED_INT_10_10_10_2 = GL_UNSIGNED_INT_10_10_10_2,
        UNSIGNED_INT_2_10_10_10_REV = GL_UNSIGNED_INT_2_10_10_10_REV
    };

    constexpr GLenum TextureUnit[]{
        GL_TEXTURE0,
        GL_TEXTURE1,
        GL_TEXTURE2,
        GL_TEXTURE3,
        GL_TEXTURE4,
        GL_TEXTURE5,
        GL_TEXTURE6,
        GL_TEXTURE7,
        GL_TEXTURE8,
        GL_TEXTURE9,
        GL_TEXTURE10,
        GL_TEXTURE11,
        GL_TEXTURE12,
        GL_TEXTURE13,
        GL_TEXTURE14,
        GL_TEXTURE15,
        GL_TEXTURE16,
        GL_TEXTURE17,
        GL_TEXTURE18,
        GL_TEXTURE19,
        GL_TEXTURE20,
        GL_TEXTURE21,
        GL_TEXTURE22,
        GL_TEXTURE23,
        GL_TEXTURE24,
        GL_TEXTURE25,
        GL_TEXTURE26,
        GL_TEXTURE27,
        GL_TEXTURE28,
        GL_TEXTURE29,
        GL_TEXTURE30,
        GL_TEXTURE31
    };

    constexpr uint32_t NumTextureUnits = sizeof(TextureUnit) / sizeof(GLenum);

    namespace {

    void glUniform(const GLint& value, GLint location)      {glUniform1i(location, value);}
    void glUniform(const GLuint& value, GLint location)     {glUniform1ui(location, value);}
    void glUniform(const GLfloat& value, GLint location)    {glUniform1f(location, value);}
    
    void glUniform(const glm::ivec2& value, GLint location) {glUniform2i(location, value[0], value[1]);}
    void glUniform(const glm::uvec2& value, GLint location) {glUniform2ui(location, value[0], value[1]);}
    void glUniform(const glm::vec2& value, GLint location)  {glUniform2f(location, value[0], value[1]);}

    void glUniform(const glm::ivec3& value, GLint location) {glUniform3i(location, value[0], value[1], value[2]);}
    void glUniform(const glm::uvec3& value, GLint location) {glUniform3ui(location, value[0], value[1], value[2]);}
    void glUniform(const glm::vec3& value, GLint location)  {glUniform3f(location, value[0], value[1], value[2]);}

    void glUniform(const glm::ivec4& value, GLint location) {glUniform4i(location, value[0], value[1], value[2], value[3]);}
    void glUniform(const glm::uvec4& value, GLint location) {glUniform4ui(location, value[0], value[1], value[2], value[3]);}
    void glUniform(const glm::vec4& value, GLint location)  {glUniform4f(location, value[0], value[1], value[2], value[3]);}

    void glUniform(const glm::mat3& value, GLint location)  {glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);}
    void glUniform(const glm::mat4& value, GLint location)  {glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);}

    template<typename T>
    constexpr DataType getGlEnumForType() {return DataType::UNKNOWN;}
    template<>
    constexpr DataType getGlEnumForType<char>() {return DataType::BYTE;}
    template<>
    constexpr DataType getGlEnumForType<unsigned char>() {return DataType::UNSIGNED_BYTE;}
    template<>
    constexpr DataType getGlEnumForType<short>() {return DataType::SHORT;}
    template<>
    constexpr DataType getGlEnumForType<unsigned short>() {return DataType::UNSIGNED_SHORT;}
    template<>
    constexpr DataType getGlEnumForType<int>() {return DataType::INT;}
    template<>
    constexpr DataType getGlEnumForType<unsigned int>() {return DataType::UNSIGNED_INT;}
    template<>
    constexpr DataType getGlEnumForType<float>() {return DataType::FLOAT;}
    template<>
    constexpr DataType getGlEnumForType<double>() {return DataType::DOUBLE;}

    }
    
    }
}

#endif // EV_GL_H