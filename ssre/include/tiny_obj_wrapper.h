/**
 * @file tiny_obj_wrapper.h
 * @author Hunter Borlik 
 * @brief tiny_obj_loader.h wrapper
 * @version 0.1
 * @date 2019-11-14
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_TINY_OBJ_WRAPPER_H
#define SSRE_TINY_OBJ_WRAPPER_H

#include <tiny_obj_loader.h>

namespace ssre::tinyobj_wrapper {

struct material_t {
tinyobj::material_t mat;
};

}

#endif // SSRE_TINY_OBJ_WRAPPER_H