#include <tree.h>
#include <game.h>

#include <skinning.hpp>

/**
 * @brief Monopodial tree-like structures of Honda
 *  based on pg. 56 of Algorithmic Botany
 * 
 */
namespace monopodial {

using namespace ptree;

constexpr uint32_t S_A      = ptree::TurtleCommands::S_A;
constexpr uint32_t S_B      = S_A + 1;
constexpr uint32_t S_C      = S_B + 1;

const std::map<std::string, float> DefaultParamsA = {
    {"R_1", 0.9f},
    {"R_2", 0.6f},
    {"a_0", ptree::degToRad(45)},
    {"a_2", ptree::degToRad(45)},
    {"d",   ptree::degToRad(137.5f)},
    {"w_r", 0.707f}
};

const std::map<std::string, float> DefaultParamsB = {
    {"R_1", 0.9f},
    {"R_2", 0.9f},
    {"a_0", ptree::degToRad(45)},
    {"a_2", ptree::degToRad(45)},
    {"d",   ptree::degToRad(137.5f)},
    {"w_r", 0.707f}
};

const std::map<std::string, float> DefaultParamsC = {
    {"R_1", 0.9f},
    {"R_2", 0.8f},
    {"a_0", ptree::degToRad(45)},
    {"a_2", ptree::degToRad(45)},
    {"d",   ptree::degToRad(137.5f)},
    {"w_r", 0.707f}
};

const std::map<std::string, float> DefaultParamsD = {
    {"R_1", 0.9f},
    {"R_2", 0.7f},
    {"a_0", ptree::degToRad(30)},
    {"a_2", ptree::degToRad(-30)},
    {"d",   ptree::degToRad(137.5f)},
    {"w_r", 0.707f}
};

constexpr Symbol<TreeSymbol> Axiom = {S_A, {2, 0.5}};

struct MonopodialProduction : public Production<TreeSymbol> {

    const float R_1   = 0.9f;             /* contraction ratio for the trunk */
    const float R_2   = 0.7f;             /* contraction ratio for the branches */
    const float a_0   = ptree::degToRad(30);     /* branching angle from the trunk */
    const float a_2   = ptree::degToRad(-30);     /* branching angle for the lateral axes */
    const float d     = ptree::degToRad(137.5f); /* divergence angle */
    const float w_r   = 0.707f;           /* width decrease rate */

    MonopodialProduction() = default;

    MonopodialProduction(float p, uint32_t sym) : Production<TreeSymbol>{p, sym} {}

    MonopodialProduction(const std::map<std::string, float> &param, float p, uint32_t sym) : Production<TreeSymbol>{p, sym},
        R_1{param.at("R_1")},
        R_2{param.at("R_2")},
        a_0{param.at("a_0")}, 
        a_2{param.at("a_2")},
        d{param.at("d")},
        w_r{param.at("w_r")}
    {
    }

    bool matches(const SymbolN<TreeSymbol>& sym) const override {
        return sym.center()->RepSym == this->A;
    }
};


struct P_1 : public MonopodialProduction {

    P_1() : MonopodialProduction{1.0f, S_A} {}
    P_1(const std::map<std::string, float> &param) : MonopodialProduction{param, 1.0f, S_A} {}

    SymbolString<TreeSymbol> translate(const SymbolN<TreeSymbol>& sym) const override {
        const TreeSymbol& value = sym.center()->value;
        float L = value.l;
        float W = value.w;
        SymbolString<TreeSymbol> ret{};
        if (matches(sym)) {
            // !(w) F(l) [ &(a0) B(l * R_2, w * w_r) ] /(d) A(l * R_1, w * w_r)
            ret.push_back({TurtleCommands::SetWidth, {W}});
            ret.push_back({TurtleCommands::S_forward, {L}});
            ret.push_back({TurtleCommands::S_push});
            ret.push_back({TurtleCommands::S_pitch, {a_0}});
            ret.push_back({S_B, {L * R_2, W * w_r}});
            ret.push_back({TurtleCommands::S_pop});
            ret.push_back({TurtleCommands::S_roll, {d}});
            ret.push_back({S_A, {L * R_1, W * w_r}});
        }
        return ret;
    }
};

struct P_2 : public MonopodialProduction {
    P_2() : MonopodialProduction{1.0f, S_B} {}
    P_2(const std::map<std::string, float> &param) : MonopodialProduction{param, 1.0f, S_B} {}

    SymbolString<TreeSymbol> translate(const SymbolN<TreeSymbol>& sym) const override {
        const TreeSymbol& value = sym.center()->value;
        float L = value.l;
        float W = value.w;
        SymbolString<TreeSymbol> ret{};
        if (matches(sym)) {
            // !(w) F(L) [ -(a_2) $ C(l * R_2, w * w_r) ] C(l * R_1, w * w_r)
            ret.push_back({TurtleCommands::SetWidth, {W}});
            ret.push_back({TurtleCommands::S_forward, {L}});
            ret.push_back({TurtleCommands::S_push});
            ret.push_back({TurtleCommands::S_yaw, {-a_2}});
            ret.push_back({TurtleCommands::S_Dollar});
            ret.push_back({S_C, {L * R_2, W * w_r}});
            ret.push_back({TurtleCommands::S_pop});
            ret.push_back({S_C, {L * R_1, W * w_r}});
        }
        return ret;
    }
};

struct P_3 : public MonopodialProduction {
    P_3() : MonopodialProduction{1.0f, S_C} {}
    P_3(const std::map<std::string, float> &param) : MonopodialProduction{param, 1.0f, S_C} {}

    SymbolString<TreeSymbol> translate(const SymbolN<TreeSymbol>& sym) const override {
        const TreeSymbol& value = sym.center()->value;
        float L = value.l;
        float W = value.w;
        SymbolString<TreeSymbol> ret{};
        if (matches(sym)) {
            // !(w) F(L) [ +(a_2) $ B(l * R_2, w * w_r) ] B(l * R_1, w * w_r)
            ret.push_back({TurtleCommands::SetWidth, {W}});
            ret.push_back({TurtleCommands::S_forward, {L}});
            ret.push_back({TurtleCommands::S_push});
            ret.push_back({TurtleCommands::S_yaw, {a_2}});
            ret.push_back({TurtleCommands::S_Dollar});
            ret.push_back({S_B, {L * R_2, W * w_r}});
            ret.push_back({TurtleCommands::S_pop});
            ret.push_back({S_B, {L * R_1, W * w_r}});
        }
        return ret;
    }
};

}

TreeNode::TreeNode(GameState* game, const std::string& name) : ev2::VisualInstance{name}, game{game} {
    buffer_layout.add_attribute(ev2::VertexAttributeType::Vertex)
        .add_attribute(ev2::VertexAttributeType::Normal)
        .add_attribute(ev2::VertexAttributeType::Color)
        .finalize();


    params = monopodial::DefaultParamsA;
}

void TreeNode::on_init() {
    ev2::VisualInstance::on_init();

    leafs = create_node<ev2::InstancedGeometry>("leafs");
    leafs->set_material_override(game->leaf_material->get_material());

    tree_geometry = ev2::renderer::Renderer::get_singleton().create_model(
        ev2::VertexBuffer::vbInitArrayVertexSpecIndexed({}, {}, buffer_layout),
        std::vector<ev2::renderer::Primitive>{},
        std::vector<ev2::renderer::Material*>{},
        glm::vec3{}, // TODO
        glm::vec3{}, // TODO
        ev2::gl::CullMode::BACK,
        ev2::gl::FrontFacing::CCW
    );
    tree_geometry->vertex_color_weight = 1.0f;
    set_model(tree_geometry);
    generate(5);
}

void TreeNode::on_destroy() {
    VisualInstance::on_destroy();

    ev2::renderer::Renderer::get_singleton().destroy_model(tree_geometry);
}

void TreeNode::setParams(const std::map<std::string, float>& paramsNew, int iterations, float growth) {
    growth_current = growth;
    auto temp_params = paramsNew;
    if (growth_current < growth_max)
    {
        temp_params["thickness"] *= growth_current;
        temp_params["w_r"] *= growth_current;
        temp_params["R_1"] *= growth_current;
        temp_params["R_2"] *= growth_current;
    }

    thickness = temp_params.at("thickness");
    params = temp_params;
    this->generate(iterations * growth_current);
    params = paramsNew; // save target parameters after generation of vertex buffers

    /*
    fruit_params.n1 = params.at("n1");
    fruit_params.n2 = params.at("n2");
    fruit_params.n3 = params.at("n3");
    fruit_params.m  = params.at("m");
    fruit_params.a  = params.at("a");
    fruit_params.b  = params.at("b");
    fruit_params.q1 = params.at("q1");
    fruit_params.q2 = params.at("q2");
    fruit_params.q3 = params.at("q3");
    fruit_params.k  = params.at("k");
    fruit_params.c  = params.at("c");
    fruit_params.d  = params.at("d_f");
    */
}

void TreeNode::generate(int iterations) {
    using namespace monopodial;
    LSystemTr<TreeSymbol> mlsys{};

    P_1 p1{params};
    P_2 p2{params};
    P_3 p3{params};

    mlsys.add_rule(&p1);
    mlsys.add_rule(&p2);
    mlsys.add_rule(&p3);

    SymbolString<TreeSymbol> str{Axiom};

    for (int i = 0; i < iterations; ++i) {
        str = mlsys.evaluate(str);
    }

    if (tree.from_symbol_string(str)) {
        // tree.simple_skeleton(5);
        tree_skeleton = tree.to_skeleton();

        leafs->instance_transforms.clear();
        leafs->instance_transforms.reserve(tree_skeleton.endpoints.size());
        const glm::mat4 rot_leaf = glm::mat4(glm::rotate<float>(glm::identity<glm::quat>(), M_PI / 2.f, glm::vec3{1, 0, 0}));
        for (const auto& ind : tree_skeleton.endpoints) {
            glm::mat4 tr = glm::translate(glm::identity<glm::mat4>(), tree_skeleton.joints[ind].position) 
                * glm::mat4(glm::quatLookAt(tree_skeleton.joints[ind].tangent, glm::vec3{0, 1, 0}))
                * rot_leaf
                * glm::translate(glm::identity<glm::mat4>(), {0, leaf_scale * -0.05, 0})
                * glm::scale(glm::identity<glm::mat4>(), glm::vec3{leaf_scale});
            leafs->instance_transforms.push_back(tr);
        }

        ptree::DefaultColorizer dc{c0, c1, tree.max_joint_depth};

        std::vector<ptree::Vertex> vertices;
        std::vector<uint32_t> indices;
        ptree::Skin_GO(5, tree_skeleton, vertices, indices, false, thickness, &dc);

        std::vector<PNVertex> g_vertices(vertices.size());
        for (int i =0; i < vertices.size(); ++i) {
            auto& sv = vertices[i];
            g_vertices[i].position = sv.pos;
            g_vertices[i].normal = sv.normal;
            g_vertices[i].color = sv.color;
        }

        tree_geometry->vertex_buffer.get_buffer(0).CopyData(g_vertices);
        tree_geometry->vertex_buffer.get_buffer(tree_geometry->vertex_buffer.get_indexed()).CopyData(indices);

        tree_geometry->primitives.clear();
        tree_geometry->primitives.push_back(ev2::renderer::Primitive{0, indices.size(), -1});
    }
}

Fruit::Fruit(const std::string& name, const SuperShapeParams& params) : ev2::VisualInstance{name}, supershape{radius_mul, 50, 50, params} {
    
}

Fruit::Fruit(const std::string& name) : ev2::VisualInstance{name}, supershape{radius_mul, 50, 50} {
    
}

void Fruit::on_init() {
    ev2::VisualInstance::on_init();

    const std::vector<uint32_t>& indices = supershape.getIndicesv();

    std::vector<ev2::renderer::Primitive> ev_meshs;
    ev_meshs.push_back(ev2::renderer::Primitive{0, indices.size(), -1});

    geometry = ev2::renderer::Renderer::get_singleton().create_model(
        ev2::VertexBuffer::vbInitSphereArrayVertexData(supershape.getInterleavedVerticesv(), indices),
        std::move(ev_meshs),
        std::vector<ev2::renderer::Material*>{},
        glm::vec3{}, // TODO
        glm::vec3{}, // TODO
        ev2::gl::CullMode::BACK,
        ev2::gl::FrontFacing::CCW);
    set_model(geometry);
}

void Fruit::generate() {
    const std::vector<uint32_t>& indices = supershape.getIndicesv();
    const std::vector<float> vbuffer = supershape.getInterleavedVerticesv();

    geometry->vertex_buffer.get_buffer(0).CopyData(vbuffer);
    geometry->vertex_buffer.get_buffer(geometry->vertex_buffer.get_indexed()).CopyData(indices);

    geometry->primitives.clear();
    geometry->primitives.push_back(ev2::renderer::Primitive{0, indices.size(), -1});
}