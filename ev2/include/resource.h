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

/**
 * @brief model id
 * 
 */
struct MID {
    MID() = default;

    bool is_valid() const noexcept {return v != -1;}

    int32_t v = -1;
private:
    friend bool operator==(const MID& a, const MID& b) noexcept {
        return a.v == b.v;
    }
};

} // namespace ev2


template<> 
struct std::hash<ev2::MID> {
    std::size_t operator()(ev2::MID const& s) const noexcept {
        std::size_t h1 = std::hash<int>{}(s.v);
        // std::size_t h2 = std::hash<int>{}(s.y);
        // return h1 ^ (h2 << 1);
        return h1;
    }
};

namespace ev2 {

class ResourceManager {
public:
    explicit ResourceManager(const std::filesystem::path& asset_path) : asset_path{asset_path}, model_lookup{} {}


    /**
     * @brief Get the model object reference id, or load object if not available
     * 
     * @param filename 
     * @return MID 
     */
    MID get_model(const std::filesystem::path& filename);
    
    
    std::filesystem::path asset_path;

private:
    std::unordered_map<std::string, MID> model_lookup;
    std::unordered_map<MID, std::shared_ptr<Model>> models;
};

std::unique_ptr<Model> loadObj(const std::filesystem::path& filename, const std::filesystem::path& base_dir);



}

#endif // EV2_RESOURCE_H