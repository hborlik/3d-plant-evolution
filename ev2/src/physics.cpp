#include <physics.h>

using namespace reactphysics3d;

namespace ev2 {

Vector3 vec_cast(glm::vec3 vec) noexcept {
    return Vector3{vec.x, vec.y, vec.z};
}

glm::vec3 react_vec_cast(Vector3 vec) noexcept {
    return glm::vec3{vec.x, vec.y, vec.z};
}

class PhysicsEventListener : public EventListener {
public:
    void onContact(const CollisionCallback::CallbackData& callbackData) override {
        std::string name0 = reinterpret_cast<Node*>(callbackData.getContactPair(0).getBody1()->getUserData())->name;
        std::string name1 = reinterpret_cast<Node*>(callbackData.getContactPair(0).getBody2()->getUserData())->name;
        // std::cout << name0 << " contact " << name1 << std::endl;
    }
};

PhysicsEventListener pel;

Physics::Physics() {
    PhysicsWorld::WorldSettings settings; 
    settings.defaultVelocitySolverNbIterations = 20;
    settings.isSleepingEnabled = false;
    settings.gravity = Vector3(0, -9.81, 0);

    world = physicsCommon.createPhysicsWorld(settings);
    world->setEventListener(&pel);
}

Physics::~Physics() {
    physicsCommon.destroyPhysicsWorld(world);
}

void Physics::simulate(double dt) {
    if (!enable_timestep)
        return;
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

std::optional<SurfaceInteraction> Physics::raycast_scene(const Ray& ray, float distance) {
    float nearest = INFINITY;
    SurfaceInteraction closest_hit{};

    class MyCallbackClass : public RaycastCallback {
    public:
        /// Hit point in world-space coordinates
        Vector3 worldPoint{};

        /// Surface normal at hit point in world-space coordinates
        Vector3 worldNormal{};

        /// Fraction distance of the hit point between point1 and point2 of the ray
        /// The hit point "p" is such that p = point1 + hitFraction * (point2 - point1)
        decimal hitFraction = 0.0f;

        /// Mesh subpart index that has been hit (only used for triangles mesh and -1 otherwise)
        int meshSubpart = 0;

        /// Hit triangle index (only used for triangles mesh and -1 otherwise)
        int triangleIndex = 0;

        /// Pointer to the hit collision body
        CollisionBody* body = nullptr;

        /// Pointer to the hit collider
        Collider* collider = nullptr;

        virtual decimal notifyRaycastHit(const RaycastInfo& info) {
            worldPoint = info.worldPoint;
            worldNormal = info.worldNormal;
            hitFraction = info.hitFraction;
            meshSubpart = info.meshSubpart;
            triangleIndex = info.triangleIndex;
            body = info.body;
            collider = info.collider;
            return decimal(info.hitFraction);
        }
    };
    MyCallbackClass nhc;
    reactphysics3d::Ray cast_ray{vec_cast(ray.origin), vec_cast(ray.eval(distance))};
    world->raycast(cast_ray, &nhc);

    if (nhc.body) { // got a hit
        closest_hit = SurfaceInteraction{
            react_vec_cast(nhc.worldNormal),
            nhc.hitFraction,
            react_vec_cast(nhc.worldPoint),
            ray.direction
        };

        closest_hit.hit = Ref<PhysicsNode>(reinterpret_cast<PhysicsNode*>(nhc.body->getUserData()));

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
    transform.rotation = glm::quat{qua.w, qua.x, qua.y, qua.z};
}

ColliderBody::ColliderBody(const std::string &name) : PhysicsNode{name} {
    body = Physics::get_singleton().get_physics_world()->createCollisionBody(get_physics_transform());
    body->setUserData(this);
}

void ColliderBody::on_init() {
    
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
    assert(collider);
    colliders.push_back(collider);
}

Ref<ColliderShape> ColliderBody::get_shape(int ind) {
    return shapes[ind];
}

// RigidBody

RigidBody::RigidBody(const std::string &name, reactphysics3d::BodyType type) : PhysicsNode{name} {
    body = Physics::get_singleton().get_physics_world()->createRigidBody(get_physics_transform());
    body->setUserData(this);
    body->setType(type);
}

void RigidBody::on_init() {
    
}

void RigidBody::on_ready() {
    body->setTransform(get_physics_transform());
}

void RigidBody::on_process(float delta) {
    
}

void RigidBody::on_destroy() {
    Physics::get_singleton().get_physics_world()->destroyRigidBody(body);
}

void RigidBody::pre_render() {
    set_cur_transform(body->getTransform());
}

void RigidBody::add_shape(Ref<ColliderShape> shape, const glm::vec3& pos) {
    shapes.push_back(shape);
    Collider* collider = body->addCollider(shape->get_shape(), reactphysics3d::Transform{Vector3{pos.x, pos.y, pos.z}, Quaternion::identity()});
    assert(collider);
    colliders.push_back(collider);
}

Ref<ColliderShape> RigidBody::get_shape(int ind) {
    return shapes[ind];
}

}