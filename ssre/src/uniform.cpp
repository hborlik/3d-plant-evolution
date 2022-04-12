/**
 * @file uniform.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-01
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <uniform.h>

using namespace ssre;

UniformBase::UniformBase(std::string name, ProgramUniformDescription location) : 
    Name{std::move(name)}, uniformDescription{std::move(location)} {

}