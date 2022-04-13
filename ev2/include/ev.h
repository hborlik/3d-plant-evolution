/**
 * @file ev.h
 * @author Hunter Borlik
 * @brief standard EV definitions. 
 * @version 0.1
 * @date 2019-09-04
 * 
 * 
 */
#ifndef EV_ENGINE_H
#define EV_ENGINE_H

#include <string>
#include <exception>

#define EV_CHECK_THROW(expr, message) if(!(expr)) throw ev2::engine_exception{"[" + std::string{__FILE__} + ":" + std::to_string(__LINE__) + "]:" + message}

namespace ev2 {

class engine_exception : public std::exception {
public:
    engine_exception(std::string description) noexcept;
    virtual ~engine_exception() = default;

    const char* what() const noexcept override;

protected:
    std::string description;
};

class shader_error : public engine_exception {
public:
    shader_error(std::string shaderName, std::string errorString) noexcept;
    virtual ~shader_error() = default;
};

void EV_init();

} // ev2

#endif // EV_ENGINE_H