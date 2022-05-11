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
#include <node.h>


namespace ev2 {

struct CID {
    int32_t v = -1;

    bool is_valid() const noexcept {
        return v > 0;
    }
};

class Physics : public Singleton<Physics> {
public:
    CID create_sphere_collider(Ref<Object> owner);
    void set_sphere_collider(CID cid, Sphere s);
    Sphere& get_sphere_collider(CID cid);
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
    int32_t next_sphere_id = 100;
};

class Collider : public Node {
public:
    explicit Collider(const std::string &name) : Node{name} {}

    void on_init() override;
    void on_ready() override;
    void on_process(float delta) override;
    void on_destroy() override;

    void set_shape(); // not implemented
    Sphere& get_shape();

private:
    CID cid{};
};

}

#endif // EV_PHYSICS_H