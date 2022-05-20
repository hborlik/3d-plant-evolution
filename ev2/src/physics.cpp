#include <physics.h>

using namespace reactphysics3d;

namespace ev2 {

Physics::Physics() {
    PhysicsWorld::WorldSettings settings; 
    settings.defaultVelocitySolverNbIterations = 20; 
    settings.isSleepingEnabled = false; 
    settings.gravity = Vector3(0, -9.81, 0);

    world = physicsCommon.createPhysicsWorld(settings);
}

Physics::~Physics() {
    physicsCommon.destroyPhysicsWorld(world);
}

void Physics::simulate(double dt) {
    // Constant physics time step 
    const float timeStep = 1.0 / 60.0; 
    double deltaTime  = dt;
    
    // Add the time difference in the accumulator 
    accumulator += deltaTime;
    
    // While there is enough accumulated time to take 
    // one or several physics steps 
    while (accumulator >= timeStep) { 
    
        // Update the physics world with a constant time step 
        world->update(timeStep); 
    
        // Decrease the accumulated time 
        accumulator -= timeStep;
    } 
    
    // Compute the time interpolation factor 
    interp_factor = accumulator / timeStep;
}

void Physics::pre_render() {

}

std::optional<SurfaceInteraction> Physics::raycast_scene(const Ray& ray) {
    float nearest = INFINITY;
    SurfaceInteraction closest_hit{};
    // for (auto& si : sphere_colldiers) {
    //     SurfaceInteraction sfi{};
    //     if (si.second.shape)
    //         if (si.second.shape->intersect(ray, sfi)) {
    //             if (sfi.t < nearest) {
    //                 nearest = sfi.t;
    //                 closest_hit = sfi;
    //                 closest_hit.hit = si.second.owner;
    //             }
    //         }
    // }
    if (closest_hit.hit.get()) {
        return {closest_hit};
    }
    return {};
}

// Collider

reactphysics3d::Transform PhysicsNode::get_physics_transform() const {
    const glm::vec3 w_pos = glm::vec3(transform.position);
    const glm::quat w_o = transform.rotation;
    const reactphysics3d::Vector3 position{w_pos.x, w_pos.y, w_pos.z};
    const reactphysics3d::Quaternion orientation{w_o.x, w_o.y, w_o.z, w_o.w};
    return reactphysics3d::Transform{position, orientation};
}

void PhysicsNode::set_cur_transform(const reactphysics3d::Transform& curr_tranform) {
    
    double factor = Physics::get_singleton().get_frame_interpolation();
    // Compute the interpolated transform of the rigid body 
    reactphysics3d::Transform interpolatedTransform = reactphysics3d::Transform::interpolateTransforms(prev_transform, curr_tranform, factor); 
    
    // Update the previous transform 
    prev_transform = curr_tranform;

    const reactphysics3d::Vector3 pos = interpolatedTransform.getPosition();
    const reactphysics3d::Quaternion qua = interpolatedTransform.getOrientation();
    transform.position = glm::vec3{pos.x, pos.y, pos.z};
    transform.rotation = glm::quat{qua.x, qua.y, qua.z, qua.w};
}

void ColliderBody::on_init() {
    body = Physics::get_singleton().get_physics_world()->createCollisionBody(get_physics_transform());
    body->setUserData(this);
}

void ColliderBody::on_ready() {
    body->setTransform(get_physics_transform());
}

void ColliderBody::on_process(float delta) {
    
}

void ColliderBody::on_destroy() {
    Physics::get_singleton().get_physics_world()->destroyCollisionBody(body);
}

void ColliderBody::pre_render() {
    // set_cur_transform(body->getTransform());
    body->setTransform(get_physics_transform());
}

void ColliderBody::add_shape(Ref<ColliderShape> shape, const glm::vec3& pos) {
    shapes.push_back(shape);
    Collider* collider = body->addCollider(shape->get_shape(), reactphysics3d::Transform{Vector3{pos.x, pos.y, pos.z}, Quaternion::identity()});

    colliders.push_back(collider);
}

Ref<ColliderShape> ColliderBody::get_shape(int ind) {
    return shapes[ind];
}

// RigidBody


void RigidBody::on_init() {
    body = Physics::get_singleton().get_physics_world()->createRigidBody(get_physics_transform());
    body->setUserData(this);
}

void RigidBody::on_ready() {
    body->setTransform(get_physics_transform());
}

void RigidBody::on_process(float delta) {
    
}

void RigidBody::on_destroy() {
    Physics::get_singleton().get_physics_world()->destroyCollisionBody(body);
}

void RigidBody::pre_render() {
    set_cur_transform(body->getTransform());
}

void RigidBody::add_shape(Ref<ColliderShape> shape, const glm::vec3& pos) {
    shapes.push_back(shape);
    Collider* collider = body->addCollider(shape->get_shape(), reactphysics3d::Transform{Vector3{pos.x, pos.y, pos.z}, Quaternion::identity()});

    colliders.push_back(collider);
}

Ref<ColliderShape> RigidBody::get_shape(int ind) {
    return shapes[ind];
}

}