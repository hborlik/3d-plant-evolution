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
#include <filesystem>

#include <ev.h>
#include <renderer/ev_gl.h>
#include <renderer/buffer.h>

namespace ev2::renderer
{

enum class VertexAttributeLabel : int {
    Vertex = 0,
    Normal,
    Color,
    Texcoord,
    Tangent,
    Bitangent
};

namespace mat_spec
{

/**
 * @brief standard shader inputs
 */
const std::string       VertexAttributeName     = "VertPos";
constexpr gl::DataType  VertexAttributeType     = gl::DataType::VEC3F;
const std::string       TextureAttributeName    = "TexPos";
constexpr gl::DataType  TextureAttributeType    = gl::DataType::VEC2F;
const std::string       NormalAttributeName     = "Normal";
constexpr gl::DataType  NormalAttributeType     = gl::DataType::VEC3F;
const std::string       VertexColorAttributeName= "VertCol";
constexpr gl::DataType  VertexColorAttributeType= gl::DataType::VEC3F;
const std::string       BiTangentAttributeName  = "BiTangent";
constexpr gl::DataType  BiTangentAttributeType  = gl::DataType::VEC3F;
const std::string       TangentAttributeName    = "Tangent";
constexpr gl::DataType  TangentAttributeType    = gl::DataType::VEC3F;

constexpr uint32_t VERTEX_BINDING_LOCATION      = 0;
constexpr uint32_t NORMAL_BINDING_LOCATION      = 1;
constexpr uint32_t COLOR_BINDING_LOCATION       = 2;
constexpr uint32_t TEXCOORD_BINDING_LOCATION    = 3;
constexpr uint32_t TANGENT_BINDING_LOCATION     = 5;
constexpr uint32_t BITANGENT_BINDING_LOCATION   = 6;
constexpr uint32_t INSTANCE_BINDING_LOCATION    = 7;

const std::unordered_map<std::string, uint32_t> AttributeBindings {
    std::make_pair("POSITION",  VERTEX_BINDING_LOCATION),
    std::make_pair("NORMAL",    NORMAL_BINDING_LOCATION),
    std::make_pair("COLOR",     COLOR_BINDING_LOCATION),
    std::make_pair("TEXCOORD_0",TEXCOORD_BINDING_LOCATION),
    std::make_pair("TANGENT",   TANGENT_BINDING_LOCATION),
    std::make_pair("BITANGENT", BITANGENT_BINDING_LOCATION)
};

inline uint32_t glBindingLocation(const std::string& attribute_name) {return AttributeBindings.at(attribute_name);}

const std::unordered_map<VertexAttributeLabel, uint32_t> DefaultBindings {
    std::make_pair(VertexAttributeLabel::Vertex,    VERTEX_BINDING_LOCATION),
    std::make_pair(VertexAttributeLabel::Normal,    NORMAL_BINDING_LOCATION),
    std::make_pair(VertexAttributeLabel::Color,     COLOR_BINDING_LOCATION),
    std::make_pair(VertexAttributeLabel::Texcoord,  TEXCOORD_BINDING_LOCATION),
    std::make_pair(VertexAttributeLabel::Tangent,   TANGENT_BINDING_LOCATION),
    std::make_pair(VertexAttributeLabel::Bitangent, BITANGENT_BINDING_LOCATION)
};

const std::unordered_map<std::string, std::tuple<uint32_t, gl::DataType, VertexAttributeLabel>> ShaderVertexAttributes {
    std::make_pair(VertexAttributeName,     std::make_tuple(VERTEX_BINDING_LOCATION,    VertexAttributeType,        VertexAttributeLabel::Vertex)),
    std::make_pair(NormalAttributeName,     std::make_tuple(NORMAL_BINDING_LOCATION,    NormalAttributeType,        VertexAttributeLabel::Normal)),
    std::make_pair(VertexColorAttributeName,std::make_tuple(COLOR_BINDING_LOCATION,     VertexColorAttributeType,   VertexAttributeLabel::Color)),
    std::make_pair(TextureAttributeName,    std::make_tuple(TEXCOORD_BINDING_LOCATION,  TextureAttributeType,       VertexAttributeLabel::Texcoord)),
    std::make_pair(TangentAttributeName,    std::make_tuple(TANGENT_BINDING_LOCATION,   TangentAttributeType,       VertexAttributeLabel::Tangent)),
    std::make_pair(BiTangentAttributeName,  std::make_tuple(BITANGENT_BINDING_LOCATION, BiTangentAttributeType,     VertexAttributeLabel::Bitangent))
};

const std::string ModelMatrixUniformName = "M";
const std::string NormalMatrixUniformName = "G";

constexpr uint32_t GUBBindingLocation = 0; // Binding location for global block
const std::string GUBName = "Globals";

// shader specific inputs
const std::string ShaderUniformBlockName = "ShaderData";

} // mat_spec

class ShaderPreprocessor {
public:
    explicit ShaderPreprocessor(const std::filesystem::path& shader_include_dir) : shader_include_dir{shader_include_dir} {}

    std::string preprocess(const std::string& input_source) const;
    void load_includes();

    std::filesystem::path get_shader_dir() const noexcept {return shader_include_dir;}

private:
    std::filesystem::path shader_include_dir;
    std::unordered_map<std::string, std::string> shader_includes; // include name to source
};

/**
 * @brief Container for GPU Shader
 */
class Shader
{
public:
    Shader(gl::GLSLShaderType type);
    ~Shader();

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    Shader(Shader &&o)
    {
        *this = std::move(o);
    }

    Shader &operator=(Shader &&o)
    {
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
     * @param pre  shader preprocessor 
     */
    void load_from(const std::filesystem::path &path, const ShaderPreprocessor& pre);

    /**
     * @brief Get the shader type
     *
     * @return ShaderType
     */
    gl::GLSLShaderType Type() const noexcept { return type; }

    bool IsCompiled() const noexcept { return compiled; }

    std::filesystem::path shaderPath() const noexcept { return path; }

    /**
     * @brief return opengl reference, for this shader program
     *
     * @return GLuint
     */
    GLuint getHandle() const noexcept { return gl_reference; }

private:
    GLuint gl_reference;
    gl::GLSLShaderType type;
    bool compiled = false;
    std::filesystem::path path;
};

class Buffer;

struct ProgramUniformDescription
{
    GLint Location = -1;
    GLenum Type = 0;
    GLint Length = 0;
};

struct ProgramInputDescription
{
    GLint Location = -1;
    GLenum Type = 0;
};

struct ProgramUniformBlockDescription
{
    GLint location = -1;  // Index in Program interface
    GLint block_size = -1; // total size in bytes of buffer

    struct Layout
    {
        GLint Offset = 0;      // offset in bytes from beginning of buffer
        GLint ArraySize = 0;   // number of array elements
        GLint ArrayStride = 0; // stride in bytes between array elements
    };
    std::unordered_map<std::string, Layout> layouts;

    Layout getLayout(const std::string &name)
    {
        auto itr = layouts.find(name);
        if (itr != layouts.end())
            return itr->second;
        return {-1, -1, -1};
    }

    GLint getOffset(const std::string &name)
    {
        auto itr = layouts.find(name);
        if (itr != layouts.end())
            return itr->second.Offset;
        return -1;
    }

    bool isValid() const noexcept {return location != -1;}

    /**
     * @brief Set a Shader parameter in the target uniform block buffer.
     * 
     * @tparam T 
     * @param paramName 
     * @param data 
     * @param shaderBuffer 
     * @return true 
     * @return false 
     */
    template <typename T>
    bool setShaderParameter(const std::string &paramName, const T &data, Buffer &shaderBuffer)
    {
        if (isValid())
        {
            GLint uoff = getOffset(paramName);
            if (uoff != -1)
            {
                shaderBuffer.SubData(data, (uint32_t)uoff);
                return true;
            }
        }
        std::cerr << "Failed to set shader parameter " << paramName << std::endl;
        return false;
    }

    template <typename T>
    bool setShaderParameter(const std::string &paramName, const std::vector<T> &data, Buffer &shaderBuffer)
    {
        if (isValid())
        {
            ProgramUniformBlockDescription::Layout layout = getLayout(paramName);
            GLint uoff = layout.Offset;
            GLint stride = layout.ArrayStride;
            if (uoff != -1 && layout.ArraySize >= data.size())
            {
                shaderBuffer.SubData(data, uoff, stride);
                return true;
            }
        }
        std::cerr << "Failed to set shader parameter " << paramName << std::endl;
        return false;
    }

    /**
     * @brief bind a given buffer as a uniform buffer
     * 
     * @param buffer 
     */
    void bind_buffer(const Buffer &buffer) {
        GL_CHECKED_CALL(
            glBindBufferRange(GL_UNIFORM_BUFFER, location, buffer.handle(), 0, buffer.size())
        );
    }
};

// GPU program specific data
class Program
{
public:
    Program();
    explicit Program(std::string name);
    ~Program();

    Program(Program &&) = delete;
    Program(const Program &) = delete;

    Program &operator=(const Program &) = delete;

    /**
     * @brief Set path for shader stage in this program
     *
     * @param program
     */
    void loadShader(gl::GLSLShaderType type, const std::filesystem::path &path, const ShaderPreprocessor& preprocessor);

    /**
     * @brief Load, Compile, and Link shader programs
     */
    void link();

    /**
     * @brief Set this program to active
     */
    void use() const;

    void unbind()
    {
        GL_CHECKED_CALL(glUseProgram(0));
    }

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
    uint32_t getModifiedCount() const noexcept { return modifiedCount; }

    /**
     * @brief True if shaders compiled and linked
     *
     * @return true
     * @return false
     */
    bool isBuilt() const noexcept { return built; }

    /**
     * @brief Get the Uniform description for given name
     *
     * @param uniformName
     * @return ProgramUniformDescription zero initialized if it does not exist
     */
    ProgramUniformDescription getUniformInfo(const std::string &uniformName) const
    {
        auto itr = uniforms.find(uniformName);
        if (itr != uniforms.end())
            return itr->second;
        return {-1, 0};
    }

    /**
     * @brief Get the Input description for given name
     *
     * @param inputName
     * @return ProgramInputDescription default initialized if it does not exist
     */
    ProgramInputDescription getInputInfo(const std::string &inputName) const
    {
        auto itr = inputs.find(inputName);
        if (itr != inputs.end())
            return itr->second;
        return {};
    }

    /**
     * @brief Get the Attribute Map. Mapping attribute index to binding location in the shader.
     * 
     * @return std::unordered_map<VertexAttributeType, int> mapping attribute identifier to binding location.
     */
    std::unordered_map<VertexAttributeLabel, uint32_t> getAttributeMap() const {
        std::unordered_map<VertexAttributeLabel, uint32_t> map;
        for (auto& mapping : inputs) {
            auto default_binding = mat_spec::ShaderVertexAttributes.find(mapping.first);
            if (default_binding != mat_spec::ShaderVertexAttributes.end()) {
                auto label = std::get<2>(default_binding->second);
                map.insert(std::make_pair(label, mapping.second.Location));
            }
        }
        return map;
    }

    /**
     * @brief Get the Uniform Block Description for shader object
     *
     * @param uboName
     * @return ProgramUniformBlockDescription zero initialized if it does not exist
     */
    ProgramUniformBlockDescription getUniformBlockInfo(const std::string &uboName) const
    {
        auto itr = uniformBlocks.find(uboName);
        if (itr != uniformBlocks.end())
            return itr->second;
        return {};
    }

    GLuint getHandle() const noexcept { return gl_reference; }

    // /**
    //  * @brief Apply a material. The program must be set active with use()
    //  * before calling this
    //  *
    //  * @param material
    //  */
    // virtual void applyMaterial(const Material &material) const;

    /**
     * @brief point uniform block with name to binding location
     *
     * @return true
     * @return false
     */
    bool setUniformBlockBinding(const std::string &uniformName, uint32_t location);

    /**
     * @brief Query the Program Resource Index for a resource of type 
     * 
     * @param resource resource type
     * @param name resource name
     * @return GLuint 
     */
    GLuint getProgramResourceLocation(GLenum resource, const std::string& name);

protected:


    // get info on program inputs
    void updateProgramInputInfo();

    // get info about program uniforms
    void updateProgramUniformInfo();

    void updateProgramUniformBlockInfo();

    /**
     * @brief Allows base classes to configure additional shader options after program builds
     *
     */
    virtual void onBuilt(){};

public:
    /**
     * @brief Program name
     */
    std::string ProgramName;

protected:
    std::unordered_map<gl::GLSLShaderType, Shader> attachedShaders;
    std::unordered_map<std::string, ProgramUniformDescription> uniforms;
    std::unordered_map<std::string, ProgramInputDescription> inputs;
    std::unordered_map<std::string, ProgramUniformBlockDescription> uniformBlocks;

    // counter for dependent objects. incremented when shader has been reloaded.
    uint32_t modifiedCount = 0;
    bool built = false;
    GLuint gl_reference;


private:
    // helper function to print program information
    friend std::ostream &operator<<(std::ostream &os, const Program &input);
};

} // namespace ev2

#endif // EV2_SHADER_H