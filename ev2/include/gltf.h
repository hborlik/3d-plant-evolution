/**
 * @file gltf.h
 * @brief 
 * @date 2022-05-18
 * 
 * 
 */
#ifndef EV2_GLTF_H
#define EV2_GLTF_H

#include <scene.h>
#include <renderer.h>

namespace ev2 {

class GLTFScene : public Scene {
public:
    explicit GLTFScene(const std::string& path) : Scene{path} {
        vertex_buffer_id = ev2::renderer::Renderer::get_singleton().create_vertex_buffer();
    }

    void on_init() override {
        Scene::on_init();
    }

    void on_destroy() override {
        Scene::on_destroy();
        for (auto& msid : meshes)
            ev2::renderer::Renderer::get_singleton().destroy_mesh(msid);
        if (vertex_buffer_id.is_valid())
            ev2::renderer::Renderer::get_singleton().destroy_vertex_buffer(vertex_buffer_id);
    }

    VertexBuffer* get_vertex_buffer() {
        return ev2::renderer::Renderer::get_singleton().get_vertex_buffer(vertex_buffer_id);
    }

    renderer::VBID get_vertex_buffer_id() const {
        return vertex_buffer_id;
    }

    void add_mesh(renderer::MSID msid) {
        meshes.push_back(msid);
    }

    renderer::MSID get_mesh(int ind) {
        return meshes[ind];
    }

private:
    renderer::VBID vertex_buffer_id;
    std::vector<renderer::MSID> meshes;
};

class GLTFNode : public Node {
public:
    explicit GLTFNode(const std::string& name) : Node{name} {
        mesh_instance_id = ev2::renderer::Renderer::get_singleton().create_mesh_instance();
    }

    void on_destroy() override {
        Node::on_destroy();
        ev2::renderer::Renderer::get_singleton().destroy_mesh_instance(mesh_instance_id);
    }

    void pre_render() override {
        ev2::renderer::Renderer::get_singleton().set_mesh_instance_transform(mesh_instance_id, get_transform());
    }
    
private:
    renderer::MSIID mesh_instance_id;
};

}

#endif // EV2_GLTF_H