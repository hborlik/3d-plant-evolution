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
    
}

void VisualInstance::on_destroy() {
    Renderer::get_singleton().destroy_instance(iid);
}

void VisualInstance::pre_render() {
    Renderer::get_singleton().set_instance_transform(iid, get_transform());
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
    
}

void CameraNode::on_destroy() {
    
}

void CameraNode::pre_render() {
    update_internal();
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
    
}

void DirectionalLightNode::on_destroy() {
    ev2::Renderer::get_singleton().destroy_light(lid);
}

void DirectionalLightNode::pre_render() {
    ev2::Renderer::get_singleton().set_light_position(lid, glm::vec3(get_transform()[3]));
}

void DirectionalLightNode::set_color(const glm::vec3& color) {
    ev2::Renderer::get_singleton().set_light_color(lid, color);
}

void DirectionalLightNode::set_ambient(const glm::vec3& color) {
    ev2::Renderer::get_singleton().set_light_ambient(lid, color);
}

// point

void PointLightNode::on_init() {
    lid = ev2::Renderer::get_singleton().create_point_light();
}

void PointLightNode::on_ready() {

}

void PointLightNode::on_process(float delta) {
    
}

void PointLightNode::on_destroy() {
    ev2::Renderer::get_singleton().destroy_light(lid);
}

void PointLightNode::pre_render() {
    ev2::Renderer::get_singleton().set_light_position(lid, glm::vec3(get_transform()[3]));
}

void PointLightNode::set_color(const glm::vec3& color) {
    ev2::Renderer::get_singleton().set_light_color(lid, color);
}

}