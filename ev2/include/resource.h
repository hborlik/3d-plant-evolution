/**
 * @file resource.h
 * @brief disk resource loader
 * @date 2022-04-18
 * 
 */
#ifndef EV2_RESOURCE_H
#define EV2_RESOURCE_H

#include <filesystem>

#include <glm/glm.hpp>

#include <material.h>
#include <mesh.h>

namespace ev2 {

class ResourceManager {
public:
    
};

std::unique_ptr<Model> loadObj(const std::string& filename, const std::string& base_dir);

}

#endif // EV2_RESOURCE_H