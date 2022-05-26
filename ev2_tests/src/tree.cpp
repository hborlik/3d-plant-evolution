#include <tree.h>

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

TreeNode::TreeNode(const std::string& name) : ev2::VisualInstance{name} {
    buffer_layout.add_attribute(ev2::VertexAttributeType::Vertex)
        .add_attribute(ev2::VertexAttributeType::Normal)
        .add_attribute(ev2::VertexAttributeType::Color)
        .finalize();
    
    model = std::make_shared<ev2::renderer::Drawable>(
        ev2::VertexBuffer::vbInitArrayVertexSpecIndexed({}, {}, buffer_layout),
        std::vector<ev2::renderer::Primitive>{},
        std::vector<ev2::renderer::Material*>{},
        glm::vec3{}, // TODO
        glm::vec3{}, // TODO
        ev2::gl::CullMode::BACK,
        ev2::gl::FrontFacing::CCW);

    model->vertex_color_weight = 1.0f;

    params = monopodial::DefaultParamsA;
}

void TreeNode::on_init() {
    ev2::VisualInstance::on_init();

    generate(5);

    tree_geometry = ev2::renderer::Renderer::get_singleton().create_model(model);
    set_model(tree_geometry);
}

void TreeNode::setParams(std::map<std::string, float> paramsNew, int iterations, float growth) {
    std::map<std::string, float> grown_params = paramsNew;
    if (growth_current < growth_max)
    {
        for (auto itr = grown_params.begin(); itr != grown_params.end(); itr++) {
            itr->second = itr->second * growth_current;
        }
    }

    params = grown_params;
    this->generate(iterations);
    params = paramsNew;
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

        model->vertex_buffer.get_buffer(0).CopyData(g_vertices);
        model->vertex_buffer.get_buffer(model->vertex_buffer.get_indexed()).CopyData(indices);

        model->primitives.clear();
        model->primitives.push_back(ev2::renderer::Primitive{0, indices.size(), -1});
    }
}