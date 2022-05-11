/**
 * @file physics.h
 * @brief 
 * @date 2022-05-10
 * 
 */
#ifndef EV_PHYSICS_H
#define EV_PHYSICS_H

#include <unordered_map>
#include <optional>

#include <ev.h>
#include <singleton.h>
#include <geometry.h>


namespace ev2 {

struct CID {
    int32_t v;

    bool is_valid() const noexcept {
        return v > 0;
    }
};

class Physics : public Singleton<Physics> {
public:
    CID create_sphere_collider(Ref<Object> owner);
    void set_sphere_collider_radius(CID cid, float r);
    void set_sphere_collider_center(CID cid, const glm::vec3& c);
    void destroy_sphere_collider(CID cid);

    void pre_render();

    std::optional<SurfaceInteraction> raycast_scene(const Ray& ray);

private:
    struct SphereInternal {
        Ref<Object> owner;
        Sphere      sphere;
    };

private:
    std::unordered_map<int32_t, SphereInternal> sphere_colldiers;
};

}

#endif // EV_PHYSICS_H