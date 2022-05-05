#include <visual_nodes.h>

#include <scene.h>

namespace ev2 {

void VisualInstance::on_init() {
    iid = Renderer::get_singleton().create_model_instance();
}

void VisualInstance::on_ready() {
    Renderer::get_singleton().set_instance_transform(iid, get_transform());
}

void VisualInstance::on_process(float delta) {
    Renderer::get_singleton().set_instance_transform(iid, get_transform());
}

void VisualInstance::on_destroy() {
    Renderer::get_singleton().destroy_instance(iid);
}

void VisualInstance::set_model(MID model) {
    Renderer::get_singleton().set_instance_model(iid, model);
}

void VisualInstance::set_material_override(int32_t material_override) {
    Renderer::get_singleton().set_instance_material_override(iid, material_override);
}

// camera

void CameraNode::on_ready() {
    
}

void CameraNode::on_process(float dt) {
    auto tr = get_transform();
    camera.set_position(glm::vec3(tr[3]));
    camera.set_rotation(glm::quat_cast(tr));
}

void CameraNode::on_destroy() {
    
}

void CameraNode::set_active() {
    get_scene().set_active_camera(Ref(this));
}

void CameraNode::update_internal() {
    auto p = glm::perspective(glm::radians(fov), aspect, near, far);
    camera.set_projection(p);
}

}