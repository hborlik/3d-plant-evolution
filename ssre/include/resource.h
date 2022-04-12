/**
 * @file resource.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-04
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_RESOURCE_H
#define SSRE_RESOURCE_H

#include <string>
#include <unordered_map>
#include <memory>

#include <singleton.h>

namespace ssre {

class Texture;
class Geometry;
class Program;
class Mesh;

class Resource : public util::Singleton<Resource> {
public:

    Resource& operator=(const Resource&) = delete;
    Resource& operator=(Resource&&) = delete;

    /**
     * @brief Get the Texture object specified by name
     * 
     * @param path 
     * @return std::shared_ptr<Texture> Null if does not exist
     */
    std::shared_ptr<Texture> getTexture(const std::string& name);

    /**
     * @brief Load a 2D texture. If texture with name already exists, that texture will be returned
     * 
     * @param path 
     * @param name 
     * @return std::shared_ptr<Texture> 
     */
    std::shared_ptr<Texture> load2DTexture(const std::string& path);

    std::shared_ptr<Texture> loadCubeTexture(const std::string& path);

    /**
     * @brief base dir should be terminated in /, path should be file in baseDir
     * 
     * @param path 
     * @param baseDir 
     * @return std::shared_ptr<Geometry> 
     */
    std::shared_ptr<Geometry> loadObj(const std::string& file, const std::string& baseDir = "");

    const std::string& getAssetPath() const noexcept {return assetPath;}

private:
    friend util::Singleton<Resource>;

    Resource(std::string assetPath);
    ~Resource();

    // textures by name
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

    // geometry by path
    std::unordered_map<std::string, std::shared_ptr<Geometry>> geometries;

    /**
     * @brief root path of assets 
     */
    const std::string assetPath;
};

}

#endif // SSRE_RESOURCE_H