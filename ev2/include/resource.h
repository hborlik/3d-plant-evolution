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

bool loadObj(std::vector<DrawObject>& drawObjects,
                       std::vector<Material> &mats,
                    //    std::map<std::string, GLuint> &textures,
                       const std::string& filename,
                       std::string base_dir);

}

#endif // EV2_RESOURCE_H