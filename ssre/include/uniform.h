/**
 * @file uniform.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-01
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_UNIFORM_H
#define SSRE_UNIFORM_H

#include <string>
#include <memory>

#include <ssre_gl.h>
#include <renderer.h>
#include <shader.h>

namespace ssre {

class UniformBase {
public:
    UniformBase(std::string name, ProgramUniformDescription location);

    UniformBase& operator=(const UniformBase&) = delete;

    std::unique_ptr<UniformBase> cloneUniform() {return std::unique_ptr<UniformBase>(cloneImpl());}

    /**
     * @brief Set the Name of the uniform variable that this value modifies
     * 
     * @param name value name
     * @param index index if value is an array type.
     */
    void setName(const std::string& name, int32_t index = -1);
    const std::string& name() const noexcept {return Name;}

    virtual void apply() = 0;

    GLuint getLocation() const noexcept {return uniformDescription.Location;}

protected:
    virtual UniformBase* cloneImpl() const = 0;

private:
    const std::string Name;
    // track state of program used to build the parameter index
    ProgramUniformDescription uniformDescription{};
};

template<typename T>
class Uniform : public UniformBase {
public:
    Uniform(std::string name, T value, ProgramUniformDescription location) :
        UniformBase{std::move(name), location}, value{std::move(value)} {
            SSRE_CHECK_THROW((GLenum)gl::getGlEnumForType<T>() == location.Type, "Uniform type is invalid for " + name + " at " + std::to_string(location.Location));
        }
    Uniform(std::string name, ProgramUniformDescription location) :
        Uniform{std::move(name), T{}, location} {
    }
    virtual ~Uniform() = default;

    std::unique_ptr<Uniform<T>> cloneUniform() {return std::unique_ptr<Uniform>(cloneImpl());}

    using value_type = T;

    void setValue(const value_type& v) {value = v;}
    const value_type& getValue() const {return value;}

    void apply() override;

protected:
    /**
     * @brief Clone Implementation
     * 
     * @return std::unique_ptr<UniformBase> 
     */
    Uniform<T>* cloneImpl() const override;
private:
    value_type value;
};

template<typename T>
Uniform<T>* Uniform<T>::cloneImpl() const {
    return new Uniform(*this);
}

template<typename T>
void Uniform<T>::apply() {
    gl::glUniform(value, getLocation());
}

}


#endif // SSRE_UNIFORM_H