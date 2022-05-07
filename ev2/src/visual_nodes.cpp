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
    update_internal();
}

void CameraNode::on_destroy() {
    
}

void CameraNode::set_active() {
    get_scene().set_active_camera(Ref(this));
}

void CameraNode::update_internal() {
    float r_aspect = ev2::Renderer::get_singleton().get_aspect_ratio();
    auto p = glm::perspective(glm::radians(fov), aspect * r_aspect, near, far);
    camera.set_projection(p);

    auto tr = get_transform();
    camera.set_position(glm::vec3(tr[3]));
    camera.set_rotation(glm::quat_cast(tr));
}

void DirectionalLightNode::on_init() {
    lid = ev2::Renderer::get_singleton().create_directional_light();
}

void DirectionalLightNode::on_ready() {

}

void DirectionalLightNode::on_process(float delta) {
    ev2::Renderer::get_singleton().set_light_position(lid, glm::vec3(get_transform()[3]));
}

void DirectionalLightNode::on_destroy() {

}

}