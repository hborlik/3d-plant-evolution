#include <physics.h>

namespace ev2 {

CID Physics::create_collider(Ref<Object> owner) {
    int32_t id = next_collider_id++;
    InternalCollider si{};
    si.owner = owner;
    auto ins = sphere_colldiers.emplace(id, si);
    if (ins.second)
        return {id};
    return {};
}

void Physics::set_collider_shape(CID cid, Ref<Shape> shape) {
    if (!cid.is_valid())
        return;
    
    auto mi = sphere_colldiers.find(cid.v);
    if (mi != sphere_colldiers.end()) {
        mi->second.shape = shape;
    }
}

Ref<Shape> Physics::get_collider(CID cid) {
    auto mi = sphere_colldiers.find(cid.v);
    if (mi != sphere_colldiers.end()) {
        return mi->second.shape;
    }
    return {};
}

void Physics::destroy_collider(CID cid) {
    sphere_colldiers.erase(cid.v);
}

void Physics::pre_render() {

}

std::optional<SurfaceInteraction> Physics::raycast_scene(const Ray& ray) {
    float nearest = INFINITY;
    SurfaceInteraction closest_hit{};
    for (auto& si : sphere_colldiers) {
        SurfaceInteraction sfi{};
        if (si.second.shape)
            if (si.second.shape->intersect(ray, sfi)) {
                if (sfi.t < nearest) {
                    nearest = sfi.t;
                    closest_hit = sfi;
                    closest_hit.hit = si.second.owner;
                }
            }
    }
    if (closest_hit.hit.get()) {
        return {closest_hit};
    }
    return {};
}

// Collider

void Collider::on_init() {
    cid = Physics::get_singleton().create_collider(get_ref());
}

void Collider::on_ready() {

}

void Collider::on_process(float delta) {
    
}

void Collider::on_destroy() {

}

void Collider::pre_render() {
    get_shape()->transform = get_transform();
}

void Collider::set_shape(Ref<Shape> shape) {
    Physics::get_singleton().set_collider_shape(cid, shape);
}

Ref<Shape> Collider::get_shape() {
    return Physics::get_singleton().get_collider(cid);
}

}