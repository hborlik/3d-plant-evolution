/**
 * @file ssre.h
 * @author Hunter Borlik
 * @brief standard SSRE library definitions. 
 * @version 0.1
 * @date 2019-09-04
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once

#include <string>
#include <exception>

#define SSRE_CHECK_THROW(expr, message) if(!(expr)) throw ssre::ssre_exception{"[" + std::string{__FILE__} + ":" + std::to_string(__LINE__) + "]:" + message}

namespace ssre {

class ssre_exception : public std::exception {
public:
    ssre_exception(std::string description) noexcept;
    virtual ~ssre_exception() = default;

    const char* what() const noexcept override;

protected:
    std::string description;
};

class ssre_shader_error : public ssre_exception {
public:
    ssre_shader_error(std::string shaderName, std::string errorString) noexcept;
    virtual ~ssre_shader_error() = default;
};

void SSRE_init();

} // ssre