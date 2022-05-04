#include <visual_nodes.h>

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

}