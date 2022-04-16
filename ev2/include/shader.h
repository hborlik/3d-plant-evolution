/**
 * @file shader.h
 * @brief GPU shader
 * @version 0.2
 * @date 2019-09-21
 * 
 * 
 */
#ifndef EV2_SHADER_H
#define EV2_SHADER_H

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include <ev.h>
#include <ev_gl.h>
#include <buffer.h>

namespace ev2 {

namespace mat_spec {

/**
 * @brief standard shader inputs
 */
constexpr const char* VertexAttributeName = "VertPos";
constexpr GLenum VertexAttributeType = GL_FLOAT_VEC3;
constexpr const char* TextureAttributeName = "TexPos";
constexpr GLenum TextureAttributeType = GL_FLOAT_VEC2;
constexpr const char* NormalAttributeName = "Normal";
constexpr GLenum NormalAttributeType = GL_FLOAT_VEC3;
constexpr const char* BiTangentAttributeName = "BiTangent";
constexpr GLenum BiNormalAttributeType = GL_FLOAT_VEC3;
constexpr const char* TangentAttributeName = "Tangent";
constexpr GLenum TangentAttributeType = GL_FLOAT_VEC3;


constexpr const char* ModelMatrixUniformName = "M";
constexpr const char* NormalMatrixUniformName = "G";

constexpr uint32_t GUBBindingLocation = 0; // Binding location for global block 
constexpr const char* GUBName = "Globals";
constexpr size_t GUBViewMatOffset = 0;
constexpr uint32_t GUBProjectionMatOffset = 64;
constexpr uint32_t GUBCameraPosOffset = 128;
constexpr uint32_t GUBNumLightsOffset = 140;
constexpr uint32_t GUBMaxNumLights = 10;
constexpr uint32_t GUBLightPositionsArrayOffset = 144;
constexpr uint32_t GUBLightPositionsArrayStride = 16;
constexpr uint32_t GUBLightPositionElementSize = 3 * sizeof(GLfloat);
constexpr uint32_t GUBLightColorsOffset = 304;
constexpr uint32_t GUBLightColorsArrayStride = 16;
constexpr uint32_t GUBLightColorElementSize = 3 * sizeof(GLfloat);
constexpr uint32_t GUBSize = 464;

// shader specific inputs
constexpr uint32_t ShaderUniformBlockBindingLocation = 1; // each shader binds its block UBO to location 1 before drawing
constexpr const char* ShaderUniformBlockName = "ShaderData";

} // mat_spec

struct Material {

};

/**
 * @brief Container for GPU Shader
 */
class Shader {
public:

    Shader(gl::GLSLShaderType type);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    
    Shader(Shader&& o) {
        *this = std::move(o);
    }

    Shader& operator=(Shader&& o) {
        gl_reference = o.gl_reference;
        o.gl_reference = 0;

        type = o.type;
        compiled = o.compiled;
        path = o.path;
        return *this;
    }

    /**
     * @brief Read compile and load a shader. Will throw ssre_exception if shader cannot be found, or cannot be compiled.
     * 
     * @param path path to shader code file
     */
    void LoadFrom(const std::string& path);

    /**
     * @brief Get the shader type
     * 
     * @return ShaderType
     */
    gl::GLSLShaderType Type() const noexcept {return type;}

    bool IsCompiled() const noexcept {return compiled;}

    std::string shaderPath() const noexcept {return path;}

    /**
     * @brief return opengl reference, for this shader program
     * 
     * @return GLuint
     */
    GLuint getHandle() const noexcept {return gl_reference;}

private:
    GLuint gl_reference;
    gl::GLSLShaderType type;
    bool compiled = false;
    std::string path;
};

class Buffer;

struct ProgramUniformDescription {
    GLint Location = -1;
    GLenum Type = 0;
    GLint Length = 0;
};

struct ProgramInputDescription {
    GLint Location = -1;
    GLenum Type = 0;
};

struct ProgramUniformBlockDescription {
    GLint Location = -1;        // Index in Program interface
    GLint BlockSize = -1;       // total size in bytes of buffer

    struct Layout {
        GLint Offset = 0;       // offset in bytes from beginning of buffer
        GLint ArraySize = 0;    // number of array elements
        GLint ArrayStride = 0;  // stride in bytes between array elements
    };
    std::unordered_map<std::string, Layout> layouts;

    Layout getLayout(const std::string& name) {
        auto itr = layouts.find(name);
        if(itr != layouts.end())
            return itr->second;
        return {-1, -1, -1};
    }

    GLint getOffset(const std::string& name) {
        auto itr = layouts.find(name);
        if(itr != layouts.end())
            return itr->second.Offset;
        return -1;
    }
};

// GPU program specific data
class Program {
public:
    explicit Program(std::string name);
    ~Program();

    Program(Program&&) = delete;
    Program(const Program&) = delete;
    
    Program& operator=(const Program&) = delete;

    /**
     * @brief Program name
     */
    const std::string ProgramName;

    /**
     * @brief Set path for shader stage in this program
     * 
     * @param program 
     */
    void loadShader(gl::GLSLShaderType type, const std::string& path);

    /**
     * @brief Load, Compile, and Link shader programs
     */
    void link();

    /**
     * @brief Set this program to active
     */
    void use() const;

    /**
     * @brief get linking status from openGL
     * 
     * @return true 
     * @return false 
     */
    bool isLinked() const;

    /**
     * @brief Get the Modified Count
     * 
     * @return uint32_t 
     */
    uint32_t getModifiedCount() const noexcept {return modifiedCount;}

    /**
     * @brief True if shaders compiled and linked
     * 
     * @return true 
     * @return false 
     */
    bool isBuilt() const noexcept {return built;}

    /**
     * @brief Get the Uniform description for given name
     * 
     * @param uniformName 
     * @return ProgramUniformDescription zero initialized if it does not exist
     */
    ProgramUniformDescription getUniformInfo(const std::string& uniformName) const {
        auto itr = uniforms.find(uniformName);
        if(itr != uniforms.end())
            return itr->second;
        return {-1, 0};
    }

    /**
     * @brief Get the Input description for given name
     * 
     * @param inputName 
     * @return ProgramInputDescription zero initialized if it does not exist
     */
    ProgramInputDescription getInputInfo(const std::string& inputName) const {
        auto itr = inputs.find(inputName);
        if(itr != inputs.end())
            return itr->second;
        return {-1, 0};
    }

    /**
     * @brief Get the Uniform Block Description for shader object
     * 
     * @param uboName 
     * @return ProgramUniformBlockDescription zero initialized if it does not exist
     */
    ProgramUniformBlockDescription getUniformBlockInfo(const std::string& uboName) const {
        auto itr = uniformBlocks.find(uboName);
        if(itr != uniformBlocks.end())
            return itr->second;
        return {};
    }

    GLuint getHandle() const noexcept {return gl_reference;}

    /**
     * @brief Apply a material. The program must be set active with use()
     * before calling this
     * 
     * @param material 
     */
    virtual void applyMaterial(const Material& material) const;

    /**
     * @brief point uniform block with name to binding location
     * 
     * @return true 
     * @return false 
     */
    bool setUniformBlockBinding(const std::string& uniformName, uint32_t location);

    /**
     * @brief Set the Shader Parameter. Value stored in this shader's data buffer
     * 
     * @return true parameter was updated
     * @return false parameter does not exist
     */
    template<typename T>
    bool setShaderParameter(const std::string& paramName, const T& data);

    template<typename T>
    bool setShaderParameter(const std::string& paramName, const std::vector<T>& data);

protected:

    std::unordered_map<gl::GLSLShaderType, Shader> attachedShaders;
    std::unordered_map<std::string, ProgramUniformDescription> uniforms;
    std::unordered_map<std::string, ProgramInputDescription> inputs;
    std::unordered_map<std::string, ProgramUniformBlockDescription> uniformBlocks;

    // shader ubo, contains shader parameters
    ProgramUniformBlockDescription shaderDataDescription;
    std::unique_ptr<Buffer> shaderUBO;

    // counter to let dependent objects know when this shader has been reloaded.
    uint32_t modifiedCount = 0;
    bool built = false;
    GLuint gl_reference;

    // get info on program inputs
    void updateProgramInputInfo();

    // get info about program uniforms
    void updateProgramUniformInfo();

    void updateProgramUniformBlockInfo();

    /**
     * @brief Allows base classes to configure additional shader options after program builds
     * 
     */
    virtual void onBuilt() {};

private:
    // helper function to print program information
    friend std::ostream& operator<< (std::ostream& os, const Program& input);
};

template<typename T>
bool Program::setShaderParameter(const std::string& paramName, const T& data) {
    if(shaderDataDescription.Location != -1) {
        GLint uoff = shaderDataDescription.getOffset(paramName);
        if(uoff != -1) {
            shaderUBO->SubData(data, uoff);
            return true;
        }
    }
    std::cerr << "Failed to set shader parameter " << paramName << " for " << ProgramName << std::endl;
    return false;
}

template<typename T>
bool Program::setShaderParameter(const std::string& paramName, const std::vector<T>& data) {
    if(shaderDataDescription.Location != -1) {
        ProgramUniformBlockDescription::Layout layout = shaderDataDescription.getLayout(paramName);
        GLint uoff = layout.Offset;
        GLint stride = layout.ArrayStride;
        if(uoff != -1 && layout.ArraySize >= data.size()) {
            shaderUBO->SubData(data, uoff, stride);
            return true;
        }
    }
    std::cerr << "Failed to set shader parameter " << paramName << " for " << ProgramName << std::endl;
    return false;
}

} // namespace ev2

#endif // EV2_SHADER_H