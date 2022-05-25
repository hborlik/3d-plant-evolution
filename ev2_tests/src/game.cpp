#include <game.h>

#include <filesystem>

#include <tree.h>
#include <physics.h>
#include <Sphere.h>

namespace fs = std::filesystem;

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

    auto bark = ev2::ResourceManager::get_singleton().create_material("bark");
    bark.first->diffuse = glm::vec3{0};
    bark.first->metallic = 0;

    tree_bark = bark;

    highlight_material = ResourceManager::get_singleton().create_material("highlight");
    highlight_material.first->diffuse = glm::vec3{0.529, 0.702, 0.478};
    highlight_material.first->sheen = 1.0;
    highlight_material.first->metallic = 0.9;

    auto ground_material = ResourceManager::get_singleton().create_material("ground_mat");
    ground_material.first->diffuse = glm::vec3{0.529, 0.702, 0.478};
    ground_material.first->sheen = 0.2;

    MID ground = ResourceManager::get_singleton().get_model( fs::path("models") / "cube.obj");
    auto g_node = scene->create_node<VisualInstance>("ground");
    g_node->set_model(ground);
    g_node->set_material_override(ground_material.second);
    g_node->transform.scale = glm::vec3{100, 0.1, 100};

    ground_plane = scene->create_node<RigidBody>("Ground Collider");
    ground_plane->add_shape(make_referenced<BoxShape>(glm::vec3{50, 0.05, 50}));
    ground_plane->add_child(g_node);
    ground_plane->transform.position = glm::vec3{0, 0, 0};

    ground_plane->get_body()->setType(reactphysics3d::BodyType::STATIC);
    auto& material = ground_plane->get_collider(0)->getMaterial();
    material.setBounciness(0.01f);

    ev2::MID hid = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "rungholt" / "house.obj");
    ev2::MID building0 = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "low_poly_houses.obj");
    ev2::MID sphere = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "sphere.obj");

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

    for (int n = 0; n < 2; n++)
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
    Renderer::get_singleton().sun_position = sun_rads;
    
    if (time_accumulator > 0.005f) {
        if (startedA && startedB)
        {
            //probably not very efficient, maybe use a queue for ungrown plants?
            for (int i = 0; i < scene->get_n_children(); i++) {
                ev2::Ref<TreeNode> tree = scene->get_child(i).ref_cast<Node>()->get_child(0).ref_cast<TreeNode>();
                if (tree) {
                    tree->growth_current = tree->growth_current + tree->growth_rate;
                    if (tree->growth_current < tree->growth_max) {
                        tree->setParams(tree->getParams(), tree->plantInfo.iterations, tree->growth_current);
                    }
                }
            }
        //Growtrees()
        }
        time_accumulator = 0.0f;
    } {
        time_accumulator += time_speed * dt / DayLength;
    }
    sun_light->transform.position = glm::rotate(glm::identity<glm::quat>(), -sun_rads, glm::vec3(1, 0, 0)) * glm::vec3{0, 0, 100};
}

void GameState::spawn_tree(const glm::vec3& position, float rotation, const std::map<std::string, float>& params, int iterations, glm::vec3 color_0, glm::vec3 color_1) {
    int unique_id = (int)randomFloatTo(9999999);
    std::string unique_hit_tag = std::string("Tree_root_") + std::to_string(unique_id);
    
    ev2::Ref<TreeNode> tree = scene->create_node<TreeNode>("Tree");
    auto debug = tree->get_parent();
    tree->plantInfo.ID = unique_id;
    tree->plantInfo.iterations = iterations;
    SuperSphere supershape(1.0f, 20, 20);
    
    ev2::Ref<ev2::RigidBody> tree_hit_sphere = scene->create_node<ev2::RigidBody>(unique_hit_tag.c_str());
    tree_hit_sphere->add_shape(ev2::make_referenced<ev2::CapsuleShape>(1.0, 5.0), glm::vec3{0, 2.5, 0});
    tree_hit_sphere->transform.position = position;
    tree_hit_sphere->transform.rotation = glm::rotate(glm::identity<glm::quat>(), rotation, glm::vec3{0, 1, 0});
    tree_hit_sphere->add_child(tree);
    tree->set_material_override(tree_bark.second);

    tree->c0 = color_0;
    tree->c1 = color_1;
    tree->setParams(params, iterations, tree->growth_current);

    if (!startedA) {
        selected_tree_1 = tree;
        startedA = true;
    } else if (!startedB) {
        selected_tree_2 = tree;
        startedB = true;
    }
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

    glm::vec3 color_0 = glm::vec3{randomFloatRange(0.1f, 1.0f), randomFloatRange(0.1f, 1.0f), randomFloatRange(0.1f, 1.0f)};
    glm::vec3 color_1 = glm::vec3{randomFloatRange(0.2f, 1.0f), randomFloatRange(0.2f, 1.0f), randomFloatRange(0.2f, 1.0f)};

    float r = sqrt(randomFloatTo(1)) * range_extent;
    float th = randomFloatTo(ptree::degToRad(360));

    glm::vec3 pos = glm::vec3{r * cos(th) , 0, r * sin(th)} + position;
    spawn_tree(pos, randomFloatTo(ptree::degToRad(360)), params, iterations, color_0, color_1);
}

void GameState::spawn_box(const glm::vec3& position) {
    ev2::MID ground = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "cube.obj");
    auto box_vis = scene->create_node<ev2::VisualInstance>("marker");
    box_vis->set_model(ground);
    box_vis->transform.scale = glm::vec3{0.5, 0.5, 0.5};
    box_vis->transform.position = glm::vec3{0, 0, 0};
    box_vis->set_material_override(highlight_material.second);

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

std::map<std::string, float> crossParams(std::map<std::string, float> paramsA, std::map<std::string, float> paramsB) {
    float randomGeneWeight = randomFloatRange(1.5f, 2.5f);
    std::map<std::string, float> retParams = {
        {"R_1", ((paramsA.find("R_1")->second + paramsB.find("R_1")->second)/randomGeneWeight)},
        {"R_2", ((paramsA.find("R_2")->second + paramsB.find("R_2")->second)/randomGeneWeight)},
        {"a_0", (randomCoinFlip(paramsA.find("a_0")->second, paramsB.find("a_0")->second))},
        {"a_2", (randomCoinFlip(paramsA.find("a_2")->second, paramsB.find("a_2")->second))},
        {"d",   (randomCoinFlip(paramsA.find("d")->second, paramsB.find("d")->second))},
        {"thickness", ((paramsA.find("thickness")->second + paramsB.find("thickness")->second)/randomGeneWeight)},
        {"w_r", ((paramsA.find("w_r")->second + paramsB.find("w_r")->second)/randomGeneWeight)}
    };
    return retParams;
}

void GameState::spawn_cross(const glm::vec3& position, float rotation, int iterations) {
    std::map<std::string, float> crossed_params = crossParams(selected_tree_1->getParams(), selected_tree_2->getParams());
    glm::vec3 color_0 = glm::vec3(randomFloatRange(selected_tree_1->c0.r, selected_tree_2->c0.r) + randomFloatRange(-.2f, .2f), randomFloatRange(selected_tree_1->c0.g, selected_tree_2->c0.g) + randomFloatRange(-.2f, .2f), randomFloatRange(selected_tree_1->c0.b, selected_tree_2->c0.b) + randomFloatRange(-.2f, .2f)); 
    glm::vec3 color_1 = glm::vec3(randomFloatRange(selected_tree_1->c1.r, selected_tree_2->c1.r) + randomFloatRange(-.2f, .2f), randomFloatRange(selected_tree_1->c1.g, selected_tree_2->c1.g) + randomFloatRange(-.2f, .2f), randomFloatRange(selected_tree_1->c1.b, selected_tree_2->c1.b) + randomFloatRange(-.2f, .2f));

    spawn_tree(position, rotation, crossed_params, iterations, color_0, color_1);
    //plantlist.push_back((Plant(unique_id, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), supershape, tree, tree_hit_sphere)));
}
