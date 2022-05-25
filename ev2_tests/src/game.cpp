#include <game.h>

#include <filesystem>

#include <tree.h>
#include <physics.h>
#include <Sphere.h>

namespace fs = std::filesystem;

float randomFloatTo(float limit) {
    return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/limit));
}

float randomFloatRange(float low, float high) {
    return rand() / static_cast<float>(RAND_MAX) * (high - low) + low;
}

GameState::GameState() {
    scene = make_referenced<Scene>("scene0");

    sun_light = scene->create_node<ev2::DirectionalLightNode>("directional_light");
    sun_light->transform.position = glm::vec3{10, 100, 0};
    sun_light->set_color(glm::vec3{5, 5, 5});
    sun_light->set_ambient({0.1, 0.1, 0.1});

    auto light = scene->create_node<ev2::PointLightNode>("point_light");
    light->transform.position = glm::vec3{0, 5, -10};
    light->set_color(glm::vec3{1, 0, 0});

    light = scene->create_node<ev2::PointLightNode>("point_light");
    light->transform.position = glm::vec3{0, 3, 10};
    light->set_color(glm::vec3{0, 1, 0});

    auto bark = ev2::ResourceManager::get_singleton().get_material("bark");
    bark->get_material()->diffuse = glm::vec3{0};
    bark->get_material()->metallic = 0;

    tree_bark = bark;

    highlight_material = ResourceManager::get_singleton().get_material("highlight");
    highlight_material->get_material()->diffuse = glm::vec3{0.529, 0.702, 0.478};
    highlight_material->get_material()->sheen = 1.0;
    highlight_material->get_material()->metallic = 0.9;

    auto ground_material = ResourceManager::get_singleton().get_material("ground_mat");
    ground_material->get_material()->diffuse = glm::vec3{0.529, 0.702, 0.478};
    ground_material->get_material()->sheen = 0.2;

    renderer::MID ground = ResourceManager::get_singleton().get_model( fs::path("models") / "cube.obj");
    auto g_node = scene->create_node<VisualInstance>("ground");
    g_node->set_model(ground);
    g_node->set_material_override(ground_material->get_material());
    g_node->transform.scale = glm::vec3{100, 0.1, 100};

    ground_plane = scene->create_node<RigidBody>("Ground Collider");
    ground_plane->add_shape(make_referenced<BoxShape>(glm::vec3{50, 0.05, 50}));
    ground_plane->add_child(g_node);
    ground_plane->transform.position = glm::vec3{0, 0, 0};

    ground_plane->get_body()->setType(reactphysics3d::BodyType::STATIC);
    auto& material = ground_plane->get_collider(0)->getMaterial();
    material.setBounciness(0.01f);

    ev2::renderer::MID hid = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "rungholt" / "house.obj");
    ev2::renderer::MID building0 = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "low_poly_houses.obj");
    ev2::renderer::MID sphere = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "sphere.obj");

    marker = scene->create_node<ev2::VisualInstance>("marker");
    marker->set_model(ground);
    marker->transform.scale = glm::vec3{0.2, 0.2, 0.2};
    marker->transform.position = glm::vec3{0, 3, 0};

    auto h_node = scene->create_node<ev2::VisualInstance>("house");
    h_node->set_model(hid);
    h_node->transform.position = glm::vec3{30, 0, 0};
    h_node->transform.rotate({0.1, 0.5, 0});
    h_node->transform.scale = glm::vec3{0.1, 0.1, 0.1};

    auto lh_node = scene->create_node<ev2::VisualInstance>("building");
    lh_node->transform.position = glm::vec3{50, 1, -20};
    lh_node->set_model(building0);

    for (int n = 0; n < 20; n++)
    {
        spawn_random_tree(glm::vec3{}, 40, 10);
    }

    spawn_player({0, 20, 0});
    cam_first_person = player->cam_first_person;
}

void GameState::update(float dt) {
    scene->update(dt);
    time_day += time_speed * dt / DayLength;
    const float sun_rads = 2.0 * M_PI * time_day;
    renderer::Renderer::get_singleton().sun_position = sun_rads;

    sun_light->transform.position = glm::rotate(glm::identity<glm::quat>(), -sun_rads, glm::vec3(1, 0, 0)) * glm::vec3{0, 0, 100};
}

void GameState::spawn_tree(const glm::vec3& position, float rotation, const std::map<std::string, float>& params, int iterations) {
    int unique_id = (int)randomFloatTo(9999999);
    std::string unique_hit_tag = std::string("Tree_root_") + std::to_string(unique_id);
    
    ev2::Ref<TreeNode> tree = scene->create_node<TreeNode>("Tree");
    
    SuperSphere supershape(1.0f, 20, 20);
    
    ev2::Ref<ev2::RigidBody> tree_hit_sphere = scene->create_node<ev2::RigidBody>(unique_hit_tag.c_str());
    tree_hit_sphere->add_shape(ev2::make_referenced<ev2::CapsuleShape>(1.0, 5.0), glm::vec3{0, 2.5, 0});
    tree_hit_sphere->transform.position = position;
    tree_hit_sphere->transform.rotation = glm::rotate(glm::identity<glm::quat>(), rotation, glm::vec3{0, 1, 0});
    tree_hit_sphere->add_child(tree);
    tree->set_material_override(tree_bark->get_material());

    tree->c0 = glm::vec3{randomFloatRange(0.1f, 1.0f), randomFloatRange(0.1f, 1.0f), randomFloatRange(0.1f, 1.0f)};
    tree->c1 = glm::vec3{randomFloatRange(0.2f, 1.0f), randomFloatRange(0.2f, 1.0f), randomFloatRange(0.2f, 1.0f)};
    tree->setParams(params, iterations);
}

void GameState::spawn_random_tree(const glm::vec3& position, float range_extent, int iterations) {
    std::map<std::string, float> params = {
        {"R_1", randomFloatRange(.6f, 1.f)},
        {"R_2", randomFloatRange(.6f, 1.f)},
        {"a_0", ptree::degToRad(randomFloatRange(12.5f, 60.f))},
        {"a_2", ptree::degToRad(randomFloatRange(12.5f, 60.f))},
        {"d",   ptree::degToRad(randomFloatRange(.0f, 360.f))},
        {"thickness", randomFloatRange(0.5f, 2.0f)},
        {"w_r", randomFloatRange(0.6f, 0.89f)}
    };

    float r = sqrt(randomFloatTo(1)) * range_extent;
    float th = randomFloatTo(ptree::degToRad(360));

    glm::vec3 pos = glm::vec3{r * cos(th) , 0, r * sin(th)} + position;
    spawn_tree(pos, randomFloatTo(ptree::degToRad(360)), params, iterations);
}

void GameState::spawn_box(const glm::vec3& position) {
    ev2::renderer::MID ground = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "cube.obj");
    auto box_vis = scene->create_node<ev2::VisualInstance>("marker");
    box_vis->set_model(ground);
    box_vis->transform.scale = glm::vec3{0.5, 0.5, 0.5};
    box_vis->transform.position = glm::vec3{0, 0, 0};
    box_vis->set_material_override(highlight_material->get_material());

    auto box = scene->create_node<ev2::RigidBody>("Box Rigidbody");
    box->add_shape(ev2::make_referenced<ev2::BoxShape>(glm::vec3{0.25, 0.25, 0.25}));
    box->add_child(box_vis);
    box->get_body()->setType(reactphysics3d::BodyType::DYNAMIC);
    box->transform.position = glm::vec3{0, 14, 0};
}

void GameState::spawn_player(const glm::vec3& position) {
    player = scene->create_node<Player>("player0", this);
    player->transform.position = position;
}