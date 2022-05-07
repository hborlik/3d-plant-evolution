#include <tree.h>

#include <skinning.hpp>

TreeNode::TreeNode(const std::string& name) : ev2::VisualInstance{name} {
    buffer_layout.add_attribute(ev2::VertexAttributeType::Vertex)
        .add_attribute(ev2::VertexAttributeType::Normal)
        .finalize();
    
    model = std::make_shared<ev2::Drawable>(
        ev2::VertexBuffer::vbInitArrayVertexSpecIndexed({}, {}, buffer_layout),
        std::vector<ev2::Mesh>{},
        glm::vec3{}, // TODO
        glm::vec3{}, // TODO
        ev2::gl::CullMode::BACK,
        ev2::gl::FrontFacing::CCW);
}

void TreeNode::on_init() {
    ev2::VisualInstance::on_init();

    generate(12);

    tree_geometry = ev2::Renderer::get_singleton().create_model(model);
    set_model(tree_geometry);
}

void TreeNode::generate(int iterations) {
    tree_skeleton = ptree::CreateSkeleton(iterations);

    std::vector<ptree::Vertex> vertices;
    std::vector<uint32_t> indices;
    ptree::Skin_GO(4, tree_skeleton, vertices, indices, true);

    std::vector<Vertex> g_vertices(vertices.size());
    for (int i =0; i < vertices.size(); ++i) {
        auto& sv = vertices[i];
        g_vertices[i].position = sv.pos;
        g_vertices[i].normal = sv.normal;
    }

    model->vertex_buffer.buffers[0].CopyData(g_vertices);
    model->vertex_buffer.buffers[model->vertex_buffer.getIndexed()].CopyData(indices);

    model->meshes.clear();
    model->meshes.push_back(ev2::Mesh{0, indices.size(), 2});
}