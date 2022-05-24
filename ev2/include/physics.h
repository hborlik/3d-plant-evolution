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

#include <reactphysics3d/reactphysics3d.h>

namespace ev2 {

class Physics : public Singleton<Physics> {
public:
    Physics();
    virtual ~Physics();

    reactphysics3d::PhysicsWorld* get_physics_world() {return world;}
    reactphysics3d::PhysicsCommon& get_physics_common() {return physicsCommon;}

    void simulate(double dt);
    void pre_render();

    std::optional<SurfaceInteraction> raycast_scene(const Ray& ray, float distance);

    double get_frame_interpolation() const {return interp_factor;}

    void enable_simulation(bool enable) noexcept {enable_timestep = enable;}

    void enable_debug_renderer(bool enable);

private:
    bool enable_timestep = true;
    double interp_factor = 0.f;
    double accumulator = 0.0f;
    int32_t next_collider_id = 100;
    reactphysics3d::PhysicsCommon physicsCommon;
    reactphysics3d::PhysicsWorld* world;
};

class ColliderShape : public Object {
public:
    virtual reactphysics3d::CollisionShape* get_shape() = 0;
};

class SphereShape : public ColliderShape {
public:
    SphereShape(float radius) {
        sphere = Physics::get_singleton().get_physics_common().createSphereShape(radius);
    }

    virtual ~SphereShape() {
        if (sphere)
            Physics::get_singleton().get_physics_common().destroySphereShape(sphere);
    }

    reactphysics3d::SphereShape* get_shape() override {
        return sphere;
    }

private:
    reactphysics3d::SphereShape* sphere = nullptr;
};

class BoxShape : public ColliderShape {
public:
    BoxShape(const glm::vec3& extent) {
        box = Physics::get_singleton().get_physics_common().createBoxShape(reactphysics3d::Vector3{extent.x, extent.y, extent.z});
    }

    virtual ~BoxShape() {
        if (box)
            Physics::get_singleton().get_physics_common().destroyBoxShape(box);
    }

    reactphysics3d::BoxShape* get_shape() override {
        return box;
    }

private:
    reactphysics3d::BoxShape* box = nullptr;
};

class CapsuleShape : public ColliderShape {
public:
    CapsuleShape(float radius, float height) {
        capsule = Physics::get_singleton().get_physics_common().createCapsuleShape(radius, height);
    }

    virtual ~CapsuleShape() {
        if (capsule)
            Physics::get_singleton().get_physics_common().destroyCapsuleShape(capsule);
    }

    reactphysics3d::CapsuleShape* get_shape() override {
        return capsule;
    }

private:
    reactphysics3d::CapsuleShape* capsule = nullptr;
};


class PhysicsNode : public Node {
public:
    explicit PhysicsNode(const std::string &name) : Node{name} {}

    /**
     * @brief Get the transform of the Physics node, note this ignores parent transforms
     *  Physics objects should be root objects in world
     * 
     * @return glm::mat4 
     */
    glm::mat4 get_transform() const override {
        glm::mat4 tr = transform.get_transform();
        return tr;
    }

protected:
    reactphysics3d::Transform get_physics_transform() const;
    void set_cur_transform(const reactphysics3d::Transform& curr_tranform);

private:
    reactphysics3d::Transform prev_transform;
};

class ColliderBody : public PhysicsNode {
public:
    explicit ColliderBody(const std::string &name);

    void on_init() override;
    void on_ready() override;
    void on_process(float delta) override;
    void on_destroy() override;

    void pre_render() override;

    void add_shape(Ref<ColliderShape> shape, const glm::vec3& pos = {});
    Ref<ColliderShape> get_shape(int ind);
    size_t get_num_shapes() const {return shapes.size();}

    reactphysics3d::Collider* get_collider(int ind) {return colliders[ind];}
    size_t get_num_colliders() const {return colliders.size();}

    reactphysics3d::CollisionBody* get_body() {return body;}

private:
    reactphysics3d::CollisionBody* body{};
    std::vector<Ref<ColliderShape>> shapes;
    std::vector<reactphysics3d::Collider*> colliders;
};

class RigidBody : public PhysicsNode {
public:
    explicit RigidBody(const std::string &name, reactphysics3d::BodyType type = reactphysics3d::BodyType::STATIC);

    void on_init() override;
    void on_ready() override;
    void on_process(float delta) override;
    void on_destroy() override;

    void pre_render() override;

    void add_shape(Ref<ColliderShape> shape, const glm::vec3& pos = {});
    Ref<ColliderShape> get_shape(int ind);
    size_t get_num_shapes() const {return shapes.size();}

    reactphysics3d::Collider* get_collider(int ind) {return colliders[ind];}
    size_t get_num_colliders() const {return colliders.size();}

    void set_mass(float mass);
    float get_mass() const;
    
    void apply_force(const glm::vec3& force);
    void apply_local_force(const glm::vec3& force);

    glm::vec3 get_velocity() const;

    void reset_force();

    reactphysics3d::RigidBody* get_body() {return body;}

private:
    reactphysics3d::RigidBody* body{};
    std::vector<Ref<ColliderShape>> shapes;
    std::vector<reactphysics3d::Collider*> colliders;
};

}

#endif // EV_PHYSICS_H