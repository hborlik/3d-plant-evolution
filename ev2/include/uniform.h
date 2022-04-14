/**
 * @file uniform.h
 * @brief 
 * @version 0.2
 * @date 2019-11-01
 * 
 * 
 */
#ifndef EV2_UNIFORM_H
#define EV2_UNIFORM_H

#include <string>
#include <memory>

#include <ev_gl.h>
// #include <renderer.h>
#include <shader.h>

namespace ev2 {

class UniformBase {
public:
    UniformBase(std::string name, ProgramUniformDescription location) : 
    Name{std::move(name)}, uniformDescription{std::move(location)} {}

    UniformBase& operator=(const UniformBase&) = delete;

    /**
     * @brief Set the Name of the uniform variable that this value modifies
     * 
     * @param name value name
     * @param index index if value is an array type.
     */
    void setName(const std::string& name, int32_t index = -1);
    const std::string& name() const noexcept {return Name;}

    virtual void apply() = 0;

    inline GLuint getLocation() const noexcept {return uniformDescription.Location;}

private:
    const std::string Name;
    // track state of program used to build the parameter index
    ProgramUniformDescription uniformDescription{};
};

template<typename T>
class Uniform : public UniformBase {
public:
    Uniform(std::string name, T value, ProgramUniformDescription location) :
        UniformBase{std::move(name), location}, value{std::move(value)}
    {
        EV_CHECK_THROW((GLenum)gl::getGlEnumForType<T>() == location.Type, "Uniform type is invalid for " + name + " at " + std::to_string(location.Location));
    }

    Uniform(std::string name, ProgramUniformDescription location) :
        Uniform{std::move(name), T{}, location} {
    }

    virtual ~Uniform() = default;

    void setValue(const T& v) {value = v;}
    const T& getValue() const {return value;}

    void apply() override;

private:
    T value;
};

template<typename T>
void Uniform<T>::apply() {
    gl::glUniform(value, getLocation());
}

}


#endif // EV2_UNIFORM_H