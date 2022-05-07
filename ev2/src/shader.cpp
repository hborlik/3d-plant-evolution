/**
 * @file shader.cpp
 * @author Hunter Borlik 
 * @brief GPU Shader
 * @version 0.1
 * @date 2019-09-22
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <algorithm>

#include <shader.h>

using namespace ev2;

namespace ev2 {

// overload stream operator
std::ostream& operator<<(std::ostream& os, const Program& input) {
    os << "=== ShaderProgram: " << input.ProgramName << " ===" << std::endl;
    for(auto& v : input.attachedShaders) {
        os << v.second.shaderPath() << std::endl;
    }
    os << "Program inputs: (name, location)" << std::endl;
    for(auto& v : input.inputs) {
        os << "    - " << v.first << ", " << v.second.Location << std::endl;
    }
    os << "Program uniforms: (name, location)" << std::endl;
    for(auto& v : input.uniforms) {
        os << "    - " << v.first << ", " << v.second.Location << std::endl;
    }
    os << "Program uniform blocks:" << std::endl;
    for(auto& v : input.uniformBlocks) {
        os << "    UB: " << v.first << " at " << v.second.location << " size: " << v.second.block_size << " bytes" << std::endl;
        for(auto& l : v.second.layouts) {
            os << "       U: " << l.first << " offset " << l.second.Offset << " len " <<l.second.ArraySize << " stride " << l.second.ArrayStride << std::endl;
        }
    }
    os << "    shader " << (input.built ? "built" : "not built") << std::endl;
    os << "=== ShaderProgram ===" << std::endl;
    return os;
}

} // ev2

std::string ShaderPreprocessor::preprocess(const std::string& input_source) const {
    using namespace std;
    ostringstream result;

    istringstream iss(input_source);

    const string include = "#include";
    size_t line_num = 0;
    for (string line; getline(iss, line); ) {
        auto pos = line.find(include);
        if (pos != string::npos) {
            size_t s = string::npos, e = string::npos;
            while (pos < line.size()) {
                if (line[pos] == '\"') {
                    if (s == string::npos) {
                        s = pos + 1;
                    } else {
                        e = pos;
                        break;
                    }
                }
                pos++;
            }
            if (s == string::npos || e == string::npos) {
                throw shader_error{"Preprocessor", std::to_string(line_num)};
            }
            string filename = line.substr(s, e - s);
            auto inc = shader_includes.find(filename);
            if (inc != shader_includes.end()) {
                result << (*inc).second;
                result << "\n";
            } else {
                throw shader_error{"Preprocessor", filename + " include not found"};
            }
        } else {
            result << line << std::endl;
        }
        line_num++;
    }
    return result.str();
}

void ShaderPreprocessor::load_includes() {
    namespace fs = std::filesystem;
    for (const auto& entry : fs::directory_iterator(shader_include_dir)) {
        const auto filenameStr = entry.path().filename().string();
        if (entry.is_directory()) {
            continue;
        }
        else if (entry.is_regular_file()) {
            if (entry.path().extension() == ".glslinc") {
                auto path = entry.path();
                std::ifstream in{path};
                // make sure the file exists
                EV2_CHECK_THROW(in.is_open(), "Shader Include File failed to open at " + path.generic_string());
                // copy out file contents
                std::string content{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
                in.close();

                shader_includes.insert_or_assign(entry.path().filename().generic_string(), content);
            }
        }
        else
            continue;
    }
}



//// SHADER ////

Shader::Shader(gl::GLSLShaderType type) : type{type} {
    gl_reference = glCreateShader((GLenum)type);
    EV2_CHECK_THROW(gl_reference != 0, "Failed to create shader");
}

Shader::~Shader() {
    if(gl_reference != 0)
        glDeleteShader(gl_reference);
}

void Shader::load_from(const std::filesystem::path& path, const ShaderPreprocessor& pre) {
    std::ifstream in{pre.get_shader_dir() / path};
    // make sure the file exists
    EV2_CHECK_THROW(in.is_open(), "Shader File not found at " + path.generic_string());
    // copy out file contents
    std::string content{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    in.close();

    content = pre.preprocess(content);

    const GLchar* codeArray = content.c_str();
    glShaderSource(gl_reference, 1, &codeArray, nullptr);
    glCompileShader(gl_reference);

    // get shader compile results
    GLint result;
    glGetShaderiv(gl_reference, GL_COMPILE_STATUS, &result);
    if(result == GL_TRUE) {
        compiled = true;
        this->path = path;
    } else { // ask for more info on failure
        std::cout << "Failed to compile shader " << path << std::endl;

        GLint logLen;
        glGetShaderiv(gl_reference, GL_INFO_LOG_LENGTH, &logLen);
        if(logLen > 0) {
            std::vector<char> log{};
            log.resize(logLen);
            
            GLsizei written;
            glGetShaderInfoLog(gl_reference, logLen, &written, log.data());

            std::cout << "Shader Log for " << path << ":\n" << std::string{log.begin(), log.end()} << std::endl;
        }
        throw shader_error{path.generic_string(), "Failed to compile shader"};
    }
}

// ShaderProgram

Program::Program() : 
    attachedShaders{} {

    gl_reference = glCreateProgram();
    if(gl_reference == 0)
        throw engine_exception{"Failed to create Shader Program"};
}

Program::Program(std::string name) : 
    ProgramName{std::move(name)}, 
    attachedShaders{} {

    gl_reference = glCreateProgram();
    if(gl_reference == 0)
        throw engine_exception{"Failed to create Shader Program"};
}

Program::~Program() {
    if(gl_reference != 0)
        glDeleteProgram(gl_reference);
}

void Program::loadShader(gl::GLSLShaderType type, const std::filesystem::path& path, const ShaderPreprocessor& preprocessor) {
    Shader s{type};
    try {
        s.load_from(path, preprocessor);
    } catch (shader_error se) {
        throw shader_error{ProgramName, se.what()};
    }
    auto suc = attachedShaders.emplace(std::pair(type, std::move(s)));
    if (!suc.second)
        throw shader_error{ProgramName, "Failed to load shader " + path.generic_string()};
}

void Program::link() {
    modifiedCount++;
    built = false;
    for(auto& stage : attachedShaders) {
        if(stage.second.IsCompiled()) {
            bool error = false;
            GL_ERROR_CHECK(glAttachShader(gl_reference, stage.second.getHandle()), error);
            if(error) {
                std::cerr << "Failed to attach shader " << stage.second.shaderPath() << " to " << ProgramName << std::endl;
                throw shader_error{ProgramName, "Failed to attach shader"};
            }
        }
    }

    glLinkProgram(gl_reference);
    // get status
    if(isLinked()) {
        // initialize lists of attributes and uniform variables
        updateProgramInputInfo();
        updateProgramUniformBlockInfo();
        updateProgramUniformInfo();
        built = true;

        // // if we have a global shader parameter ubo, save it
        // // TODO separate global shader information UBO from shader data UBO
        // shaderGlobalDataDescription = getUniformBlockInfo(mat_spec::GUBName);
        // if (shaderGlobalDataDescription.isValid()) {
        //     shaderParameters = std::make_unique<Buffer>(gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW);
        //     shaderParameters->Allocate(shaderGlobalDataDescription.block_size);
        // }

        // additional configuration
        onBuilt();
    } else {
        std::cerr << "Failed to link program " << ProgramName << std::endl;
        
        GLint logLen;
        glGetProgramiv(gl_reference, GL_INFO_LOG_LENGTH, &logLen);
        if(logLen > 0) {
            std::vector<char> log{};
            log.resize(logLen);
            
            GLsizei written;
            glGetProgramInfoLog(gl_reference, logLen, &written, log.data());

            std::cerr << "Program Log for " << ProgramName << ":\n" << std::string{log.begin(), log.end()} << std::endl;
        }
        throw shader_error{ProgramName, "Failed to link program"};
    }
}

void Program::use() const {
    // set program to active
    glUseProgram(gl_reference);
}

void Program::applyMaterial(const Material& material) const {
    // const auto& textureInputs = material->getTextureInputs();
    // for(uint32_t i = 0; i < textureInputs.size(); i++) { 
    //     if(textureInputs[i].texture) {
    //         // set and activate texture unit for editing
    //         glActiveTexture(gl::TextureUnit[i]);
    //         textureInputs[i].texture->bind();
    //         // update sampler uniform for texture
    //         glUniform1i(textureInputs[i].location, i);
    //     }
    // }

    int loc = getUniformInfo("ambient").Location;
    if (loc != -1) {
        GL_CHECKED_CALL(ev2::gl::glUniform(material.ambient, loc));
    }

    loc = getUniformInfo("diffuse").Location;
    if (loc != -1) {
        GL_CHECKED_CALL(ev2::gl::glUniform(material.diffuse, loc));
    }

    loc = getUniformInfo("specular").Location;
    if (loc != -1) {
        GL_CHECKED_CALL(ev2::gl::glUniform(material.specular, loc));
    }

    loc = getUniformInfo("transmittance").Location;
    if (loc != -1) {
        GL_CHECKED_CALL(ev2::gl::glUniform(material.transmittance, loc));
    }

    loc = getUniformInfo("emission").Location;
    if (loc != -1) {
        GL_CHECKED_CALL(ev2::gl::glUniform(material.emission, loc));
    }

    loc = getUniformInfo("shininess").Location;
    if (loc != -1) {
        GL_CHECKED_CALL(ev2::gl::glUniformf(material.shininess, loc));
    }

}

bool Program::isLinked() const {
    GLint status;
    glGetProgramiv(gl_reference, GL_LINK_STATUS, &status);
    return status == GL_TRUE;
}

bool Program::setUniformBlockBinding(const std::string& uniformName, uint32_t location) {
    int loc = getUniformBlockInfo(uniformName).location;
    if(loc != -1) {
        glUniformBlockBinding(gl_reference, loc, location);
        return true;
    }
    return false;
}

void Program::updateProgramInputInfo() {
    GLint numAttribs = 0;
    glGetProgramInterfaceiv(gl_reference, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);
    const GLenum properties[3] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION};

    inputs.clear();

    for(int attr = 0; attr < numAttribs; ++attr) {
        GLint values[3];
        glGetProgramResourceiv(gl_reference, GL_PROGRAM_INPUT, attr, 3, properties, 3, NULL, values);

        // Get the name
        std::vector<char> name{};
        name.resize(values[0]);
        glGetProgramResourceName(gl_reference, GL_PROGRAM_INPUT, attr, name.size(), NULL, name.data());
        name.erase(std::find(name.begin(), name.end(), '\0'), name.end());

        // save attr information
        std::string pname{name.begin(), name.end()};
        ProgramInputDescription& pid = inputs[pname];
        pid.Location = values[2];
        pid.Type = values[1];
    }
}

void Program::updateProgramUniformInfo() {
    GLint numUniforms = 0;
    glGetProgramInterfaceiv(gl_reference, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
    const GLenum properties[4] = {GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_LOCATION};

    uniforms.clear();

    for(uint32_t unif = 0; unif < numUniforms; ++unif) {
        GLint values[4];
        glGetProgramResourceiv(gl_reference, GL_UNIFORM, unif, 4, properties, 4, NULL, values);

        // Skip any uniforms that are in a block.
        if(values[0] != -1)
            continue;


        // get Length of uniform array
        GLuint props[]  = {GL_UNIFORM_SIZE};
        GLuint unifs[]  = {unif};
        GLint rets[]    = {0};
        glGetActiveUniformsiv(gl_reference, 1, unifs, GL_UNIFORM_SIZE, rets);

        // Get the name
        std::vector<char> name{};
        name.resize(values[2]);
        glGetProgramResourceName(gl_reference, GL_UNIFORM, unif, name.size(), NULL, name.data());
        name.erase(std::find(name.begin(), name.end(), '\0'), name.end());

        // save uniform information
        std::string uname{name.begin(), name.end()};
        ProgramUniformDescription& pud = uniforms[uname]; // reference uniform description
        pud.Type        = values[1];
        pud.Location    = values[3];
        pud.Length      = rets[0];
    }
}

void Program::updateProgramUniformBlockInfo() {
    GLint numBlocks = 0;
    glGetProgramInterfaceiv(gl_reference, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);
    const GLenum blockProperties[1] = {GL_NUM_ACTIVE_VARIABLES};
    const GLenum activeUnifProp[1] = {GL_ACTIVE_VARIABLES};

    uniformBlocks.clear();

    for(int blockIx = 0; blockIx < numBlocks; ++blockIx) {
        GLint numActiveVars = 0;
        glGetProgramResourceiv(gl_reference, GL_UNIFORM_BLOCK, blockIx, 1, blockProperties, 1, NULL, &numActiveVars);

        if(!numActiveVars)
            continue;

        // get size of uniform block
        GLint blockSize;
        glGetActiveUniformBlockiv(gl_reference, blockIx, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

        // get the binding of the uniform block
        GLint binding;
        glGetActiveUniformBlockiv(gl_reference, blockIx, GL_UNIFORM_BLOCK_BINDING, &binding);

        // get the uniform block name
        GLint nameLength;
        glGetActiveUniformBlockiv(gl_reference, blockIx, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);
        std::vector<char> namebuffer{};
        namebuffer.resize(nameLength);
        glGetActiveUniformBlockName(gl_reference, blockIx, nameLength, nullptr, namebuffer.data());

        std::string blockName{namebuffer.begin(), namebuffer.end()};
        blockName.erase(std::find(blockName.begin(), blockName.end(), '\0'), blockName.end());

        ProgramUniformBlockDescription& pubd = uniformBlocks[blockName];
        pubd.location = binding;
        pubd.block_size = blockSize;

        std::vector<GLuint> unifIndices(numActiveVars); // array of active variable indices associated with an active uniform block
        glGetProgramResourceiv(gl_reference, GL_UNIFORM_BLOCK, blockIx, 1, activeUnifProp, numActiveVars, NULL, (GLint*)unifIndices.data());

        std::vector<GLint> offsets(numActiveVars);
        glGetActiveUniformsiv(gl_reference, numActiveVars, unifIndices.data(), GL_UNIFORM_OFFSET, offsets.data());

        std::vector<GLint> strides(numActiveVars); // stride between consecutive elements in array
        glGetActiveUniformsiv(gl_reference, numActiveVars, unifIndices.data(), GL_UNIFORM_ARRAY_STRIDE, strides.data());
        // for(int i = 0; i < numActiveVars; ++i) {
        //     std::cout << strides[i] << std::endl;
        // }

        std::vector<GLint> sizes(numActiveVars); // number of items in uniform, greater than 1 for arrays
        glGetActiveUniformsiv(gl_reference, numActiveVars, unifIndices.data(), GL_UNIFORM_SIZE, sizes.data());

        // for each uniform in block
        for(int unifIx = 0; unifIx < numActiveVars; ++unifIx) {
            const GLenum unifProperties[3] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION};
            GLint values[3];
            glGetProgramResourceiv(gl_reference, GL_UNIFORM, unifIndices[unifIx], 3, unifProperties, 3, NULL, values);

            // Get the name
            std::string name{};
            name.resize(values[0]);
            glGetProgramResourceName(gl_reference, GL_UNIFORM, unifIndices[unifIx], name.size(), NULL, &name[0]);
            name.erase(std::find(name.begin(), name.end(), '\0'), name.end());

            //std::cout << "UB: " << name << ", GL_UNIFORM_BLOCK " << unifIndices[unifIx] << ", Location " << values[2] << std::endl;

            pubd.layouts[name] = ProgramUniformBlockDescription::Layout{offsets[unifIx], sizes[unifIx], strides[unifIx]};

        }

    }
}