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

    auto light = scene->create_node<ev2::PointLightNode>("point_light");
    light->transform.position = glm::vec3{0, 5, -10};
    light->set_color(glm::vec3{1, 0, 0});

    light = scene->create_node<ev2::PointLightNode>("point_light");
    light->transform.position = glm::vec3{0, 3, 10};
    light->set_color(glm::vec3{0, 1, 0});

    auto bark = ev2::ResourceManager::get_singleton().get_material("bark");
    bark->get_material()->diffuse = glm::vec3{};
    bark->get_material()->metallic = 0;
    bark->get_material()->subsurface = 0.05;
    bark->get_material()->specular = 0.08;
    bark->get_material()->roughness = 0.75;
    bark->get_material()->specularTint = 0.1;
    bark->get_material()->clearcoat = 0.0;
    bark->get_material()->clearcoatGloss = 0.0;
    bark->get_material()->sheen = 0.1;
    bark->get_material()->sheenTint = 0.54;

    tree_bark = bark;

    auto light_material = ResourceManager::get_singleton().get_material("light_material");
    light_material->get_material()->diffuse = glm::vec3{};
    light_material->get_material()->emissive = glm::vec3{10, 10, 10};

    highlight_material = ResourceManager::get_singleton().get_material("highlight");
    highlight_material->get_material()->diffuse = glm::vec3{};
    highlight_material->get_material()->emissive = glm::vec3{10, 0, 0};
    highlight_material->get_material()->sheen = 1.0;
    highlight_material->get_material()->metallic = 0.9;

    default_fruit_material = ResourceManager::get_singleton().get_material("fruit_material");
    // fruit_material->get_material()->emissive = glm::vec3{2.29, 6.02, 4.78};
    default_fruit_material->get_material()->diffuse = glm::vec3{0.229, 0.602, 0.478};
    default_fruit_material->get_material()->sheen = 0.7;
    default_fruit_material->get_material()->roughness = 0.4f;
    default_fruit_material->get_material()->clearcoat = 0.2f;
    default_fruit_material->get_material()->metallic = 0.0;

    default_leaf_material = ResourceManager::get_singleton().get_material("leaf_material");
    default_leaf_material->get_material()->diffuse = glm::vec3{165/255.0, 17/255.0, 177/255.0};
    default_leaf_material->get_material()->emissive = 1.5f * glm::vec3{37/255.0, 0/255.0, 255/255.0};
    default_leaf_material->get_material()->metallic = 0.09f;
    default_leaf_material->get_material()->subsurface = 1.0f;
    default_leaf_material->get_material()->specular = 0.04;
    default_leaf_material->get_material()->roughness = 0.4;
    default_leaf_material->get_material()->specularTint = 0.f;
    default_leaf_material->get_material()->clearcoat = 0.88;
    default_leaf_material->get_material()->clearcoatGloss = 0.63;
    default_leaf_material->get_material()->sheen = 0.35f;
    default_leaf_material->get_material()->sheenTint = 0.5f;
    default_leaf_material->get_material()->diffuse_tex = ResourceManager::get_singleton().get_texture("coffee_leaf1.png");

    auto ground_material = ResourceManager::get_singleton().get_material("ground_mat");
    ground_material->get_material()->metallic = 0.16;
    ground_material->get_material()->subsurface = 0.95;
    ground_material->get_material()->specular = 0.0;
    ground_material->get_material()->roughness = 0.77;
    ground_material->get_material()->specularTint = 0.25;
    ground_material->get_material()->clearcoat = 0.29;
    ground_material->get_material()->clearcoatGloss = 0.8;
    ground_material->get_material()->sheen = 0.43;
    ground_material->get_material()->sheenTint = 0.5;
    ground_material->get_material()->diffuse = glm::vec3{22/255.0, 116/255.0, 34/255.0};


    auto ground = ResourceManager::get_singleton().get_model( fs::path("models") / "cube.obj");
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

    auto hid = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "rungholt" / "house.obj");
    auto building0 = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "house" / "house.obj");
    for (auto& m : building0->materials) {
        m->roughness = 0.26f;
        m->clearcoat = 0.03f;
        m->clearcoatGloss = 1.0f;
        m->sheen = 0.77f;
    }

    auto sphere = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "sphere.obj");
    auto wagon = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "Wagon.obj");

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
    lh_node->transform.position = glm::vec3{30, 0, -10};
    lh_node->set_model(building0);

    auto w_node = scene->create_node<ev2::VisualInstance>("Wagon");
    w_node->transform.position = glm::vec3{0, 0.43, -20};
    w_node->transform.rotate(glm::vec3{0, 0.2, -0.3});
    w_node->transform.scale = glm::vec3{0.2};
    w_node->set_model(wagon);

    auto flies = scene->create_node<FireFlies>(this, "flies", 100);

    // auto instance_node = scene->create_node<ev2::InstancedGeometry>("instance_test");
    // instance_node->instance_transforms.push_back(glm::translate(glm::identity<glm::mat4>(), {0, 10, 0}));
    // instance_node->instance_transforms.push_back(glm::translate(glm::identity<glm::mat4>(), {1, 10, 0}));
    // instance_node->instance_transforms.push_back(glm::translate(glm::identity<glm::mat4>(), {2, 10, 0}));
    // instance_node->instance_transforms.push_back(glm::translate(glm::identity<glm::mat4>(), {3, 10, 0}));

    for (int n = 0; n < 20; n++)
    {
        spawn_random_tree(glm::vec3{}, 40, 9, 1.0f);
    }

    // for (int x = -50; x < 50; x+=10)
    // {
    //     spawn_mountain_tree(glm::vec3{x, 0, 50}, 40, 8);
    //     spawn_mountain_tree(glm::vec3{x, 0, -50}, 40, 8);
    // }
    // for (int z = -50; z < 50; z+=10)
    // {
    //     spawn_mountain_tree(glm::vec3{50, 0, z}, 40, 8);
    //     spawn_mountain_tree(glm::vec3{-50, 0, z}, 40, 8);
    // }    
    spawn_player({0, 20, 0});
    cam_first_person = player->cam_first_person;
}

void GameState::update(float dt) {
    scene->update(dt);
    time_day += time_speed * dt / DayLength;
    const float sun_rads = 2.0 * M_PI * time_day;
    
    int j = 0;
    for (ev2::Ref<Node> node : scene->get_children()) {
        ev2::Ref<TreeNode> tree = node.ref_cast<Node>()->get_child(0).ref_cast<TreeNode>();
        // if (j < 3){
            if (tree) {
                if (tree->growth_current < tree->growth_max) {
                    tree->growth_current = tree->growth_current + tree->growth_rate * dt * (1/(log(tree->growth_current + 1.1f)));
                    tree->setParams(tree->getParams(), tree->plantInfo.iterations, tree->growth_current);
                    j++;
                }

            }
        // } else {
        //     break;
        // }
    }
    renderer::Renderer::get_singleton().sun_position = sun_rads;

    float sun_brightness = std::pow(std::max<float>(sin(sun_rads), 0), 0.33);
    float sun_scatter = .1f * std::pow(std::max<float>(cos(2 * sun_rads),0), 5);

    sun_light->set_color(glm::vec3{5, 5, 5} * sun_brightness + sunset_color * sun_scatter);
    sun_light->set_ambient(glm::vec3{0.05, 0.05, 0.05} * sun_brightness + sunset_color * sun_scatter + (1 - sun_brightness) * night_ambient * .4f);

    sun_light->transform.position = glm::rotate(glm::identity<glm::quat>(), -sun_rads, glm::vec3(1, 0, 0)) * glm::vec3{0, 0, 100};
}

void GameState::spawn_tree(const glm::vec3& position, float rotation, const std::map<std::string, float>& params, int iterations, 
                                 glm::vec3 color_0, glm::vec3 color_1, float starting_growth, float adjusted_leaf_scale, 
                                 ev2::Ref<ev2::MaterialResource> new_leaf_material, ev2::Ref<ev2::MaterialResource> new_fruit_material, 
                                 float fruit_spawn_rate, bool breedable) {
    int unique_id = (int)randomFloatTo(9999999);
    std::string unique_hit_tag = std::string("Tree_root_") + std::to_string(unique_id);
    
    ev2::Ref<TreeNode> tree = scene->create_node<TreeNode>(this, "Tree", breedable, unique_id, new_fruit_material, new_leaf_material);
    auto debug = tree->get_parent();
    tree->plantInfo.ID = unique_id;
    tree->breedable = breedable;
    tree->plantInfo.iterations = iterations;
    SuperSphere supershape(1.0f, 20, 20);
    tree->growth_current = starting_growth;
    tree->leaf_scale = adjusted_leaf_scale;

    ev2::Ref<ev2::RigidBody> tree_hit_sphere = scene->create_node<ev2::RigidBody>(unique_hit_tag.c_str());
    tree_hit_sphere->add_shape(ev2::make_referenced<ev2::CapsuleShape>(.5 * params.find("thickness")->second/2, 5.0), glm::vec3{0, 2.5, 0});
    tree_hit_sphere->transform.position = position;
    tree_hit_sphere->transform.rotation = glm::rotate(glm::identity<glm::quat>(), rotation, glm::vec3{0, 1, 0});
    tree_hit_sphere->add_child(tree);
    tree->set_material_override(tree_bark->get_material());
    if (breedable)
    {
        auto light_material = ResourceManager::get_singleton().get_material("light_material");
        auto cube = ResourceManager::get_singleton().get_model( fs::path("models") / "cube.obj");
        auto light_geom = scene->create_node<VisualInstance>("ground");
        light_geom->set_model(cube);
        light_geom->set_material_override(light_material->get_material());
        light_geom->transform.scale = glm::vec3{0.1, 0.5, 0.1};

        auto light = scene->create_node<ev2::PointLightNode>("point_light");
        light->transform.position = glm::vec3{1, 0.3, 0};
        light->set_color(color_0 * 3.f);
        light->add_child(light_geom);

        tree_hit_sphere->add_child(light);

        tree->fruit_spawn_rate = fruit_spawn_rate;

        //spawn_fruit(position + glm::vec3{0, 10, 0}, tree->fruit_params);
    } else {
        tree->fruits_spawned = true;
    }
    tree->c0 = color_0;
    tree->c1 = color_1;
    tree->setParams(params, iterations, tree->growth_current);
    
}

void GameState::spawn_random_tree(const glm::vec3& position, float range_extent, int iterations, float starting_growth) {
    std::map<std::string, float> params = {
        {"R_1", randomFloatRange(.6f, 1.0f)},
        {"R_2", randomFloatRange(.6f, 1.f)},
        {"a_0", ptree::degToRad(randomFloatRange(12.5f, 60.f))},
        {"a_2", ptree::degToRad(randomFloatRange(12.5f, 60.f))},
        {"d",   ptree::degToRad(randomFloatRange(.0f, 720.f))},
        {"thickness", randomFloatRange(0.5f, 2.0f)},
        {"w_r", randomFloatRange(0.6f, 0.89f)},
        // fruit params
        {"n1",  randomFloatRange(.2f, .5f)},
        {"n2",  randomFloatRange(.9f, 2.f)},
        {"n3",  randomFloatRange(.9f, 2.f)},
        {"m",   (int)randomFloatRange(1, 7.f)},
        {"a",   randomFloatRange(.99f, 1.05f)},
        {"b",   randomFloatRange(.99f, 1.05f)},

        {"q1",  randomFloatRange(.2f, .5f)},
        {"q2",  randomFloatRange(.9f, 2.f)},
        {"q3",  randomFloatRange(.9f, 2.f)},
        {"k",   (int)randomFloatRange(1, 3.f)},
        {"c",   randomFloatRange(.99f, 1.05f)},
        {"d_f", randomFloatRange(.99f, 1.05f)},
    };



    glm::vec3 color_0 = glm::vec3{randomFloatRange(0.1f, 1.0f), randomFloatRange(0.1f, 1.0f), randomFloatRange(0.1f, 1.0f)};
    glm::vec3 color_1 = glm::vec3{randomFloatRange(0.2f, 1.0f), randomFloatRange(0.2f, 1.0f), randomFloatRange(0.2f, 1.0f)};

    float r = sqrt(randomFloatTo(1)) * range_extent;
    float th = randomFloatTo(ptree::degToRad(360));
    float random_leaf_scale = randomFloatRange(0.5f, 15.f);
    glm::vec3 pos = glm::vec3{r * cos(th) , 0, r * sin(th)} + position;
//    glm::vec3 pos = position;

    ev2::Ref<ev2::MaterialResource> leaf_material = ev2::make_referenced<ev2::MaterialResource>(ev2::renderer::Renderer::get_singleton().create_material());
    leaf_material->get_material()->diffuse = glm::vec3{randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.)};
    leaf_material->get_material()->emissive = randomFloatRange(0.0001, 1.) * glm::vec3{randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.)};
    leaf_material->get_material()->metallic = randomFloatRange(0.0001, 0.3);
    leaf_material->get_material()->subsurface = randomFloatRange(0.5, 1);
    leaf_material->get_material()->specular = randomFloatRange(0.0001, 0.2);;
    leaf_material->get_material()->roughness = randomFloatRange(0.0001, 1);;
    leaf_material->get_material()->specularTint = 0.f;
    leaf_material->get_material()->clearcoat = randomFloatRange(0.0001, 1.0);;
    leaf_material->get_material()->clearcoatGloss = 0.63;
    leaf_material->get_material()->sheen = randomFloatRange(0.0001, 0.8);;
    leaf_material->get_material()->sheenTint = 0.5f;
    leaf_material->get_material()->diffuse_tex = ResourceManager::get_singleton().get_texture("coffee_leaf1.png");

    std::string fruit_mat = std::to_string((int)randomFloatTo(99999999)).append("fruit_material");
    ev2::Ref<ev2::MaterialResource> fruit_material = ResourceManager::get_singleton().get_material(fruit_mat);
    fruit_material->get_material()->diffuse = glm::vec3{randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.)};
    fruit_material->get_material()->sheen = randomFloatRange(0.2, 1.0);;
    fruit_material->get_material()->roughness = randomFloatRange(0.0001, 1.0);
    fruit_material->get_material()->clearcoat = 0.2f;
    fruit_material->get_material()->metallic = randomFloatRange(0.0001, 0.5);
    fruit_material->get_material()->emissive = randomFloatRange(0.0001, 1.) * fruit_material->get_material()->diffuse;
    float fruit_spawn_rate = randomFloatRange(0.001f, 0.5);

    spawn_tree(pos, randomFloatTo(ptree::degToRad(360)), params, (int)randomFloatRange(3, 10), color_0, color_1, starting_growth, random_leaf_scale, leaf_material, fruit_material, fruit_spawn_rate, true);
}


void GameState::spawn_mountain_tree(const glm::vec3& position, float range_extent, int iterations) {
    std::map<std::string, float> params = {
        {"R_1", randomFloatRange(.6f, 1.0f)},
        {"R_2", randomFloatRange(.6f, 1.f)},
        {"a_0", ptree::degToRad(randomFloatRange(12.5f, 60.f))},
        {"a_2", ptree::degToRad(randomFloatRange(12.5f, 60.f))},
        {"d",   ptree::degToRad(randomFloatRange(.0f, 360.f))},
        {"thickness", randomFloatRange(40.5f, 75.0f)},
        {"w_r", randomFloatRange(0.6f, 0.89f)}
    };

    glm::vec3 color_0 = glm::vec3{randomFloatRange(0.1f, 1.0f), randomFloatRange(0.1f, 1.0f), randomFloatRange(0.1f, 1.0f)};
    glm::vec3 color_1 = glm::vec3{randomFloatRange(0.2f, 1.0f), randomFloatRange(0.2f, 1.0f), randomFloatRange(0.2f, 1.0f)};

    float r = sqrt(randomFloatTo(1)) * range_extent;
    float th = randomFloatTo(ptree::degToRad(360));

//    glm::vec3 pos = glm::vec3{r * cos(th) , 0, r * sin(th)} + position;
    glm::vec3 pos = position;

    spawn_tree(pos, randomFloatTo(ptree::degToRad(360)), params, iterations, color_0, color_1, 1.f, 0.0f, default_leaf_material, default_fruit_material, -1.0f, false);
}

void GameState::spawn_box(const glm::vec3& position) {
    auto ground = ev2::ResourceManager::get_singleton().get_model( fs::path("models") / "cube.obj");
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

std::map<std::string, float> crossParams(std::map<std::string, float> paramsA, std::map<std::string, float> paramsB) {
    float randomGeneWeight = randomFloatTo(1.f);
    std::map<std::string, float> retParams = {
        {"R_1", (paramsA.find("R_1")->second * (1 - randomGeneWeight) + paramsB.find("R_1")->second * randomGeneWeight)},
        {"R_2", (paramsA.find("R_2")->second * (1 - randomGeneWeight) + paramsB.find("R_2")->second * randomGeneWeight)},
        {"a_0", (randomCoinFlip(paramsA.find("a_0")->second, paramsB.find("a_0")->second))},
        {"a_2", (randomCoinFlip(paramsA.find("a_2")->second, paramsB.find("a_2")->second))},
        {"d",   (randomCoinFlip(paramsA.find("d")->second, paramsB.find("d")->second))},
        {"thickness", (paramsA.find("thickness")->second * (1 - randomGeneWeight) + paramsB.find("thickness")->second * randomGeneWeight)},
        {"w_r", (paramsA.find("w_r")->second * (1 - randomGeneWeight) + paramsB.find("w_r")->second * randomGeneWeight)}
    };
    return retParams;
}


void GameState::spawn_cross(const glm::vec3& position, float rotation, int iterations) {
    if (selected_tree_1 && selected_tree_2) {
        std::map<std::string, float> crossed_params = crossParams(selected_tree_1->getParams(), selected_tree_2->getParams());
        SuperShapeParams cross_fruit_params;

        cross_fruit_params.a = (selected_tree_1->fruit_params.a + selected_tree_2->fruit_params.a)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.b = (selected_tree_1->fruit_params.b + selected_tree_2->fruit_params.b)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.c = (selected_tree_1->fruit_params.c + selected_tree_2->fruit_params.c)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.d = (selected_tree_1->fruit_params.d + selected_tree_2->fruit_params.d)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.k = (selected_tree_1->fruit_params.k + selected_tree_2->fruit_params.k)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.m = (selected_tree_1->fruit_params.m + selected_tree_2->fruit_params.m)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.n1 = (selected_tree_1->fruit_params.n1 + selected_tree_2->fruit_params.n1)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.n2 = (selected_tree_1->fruit_params.n2 + selected_tree_2->fruit_params.n2)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.n3 = (selected_tree_1->fruit_params.n3 + selected_tree_2->fruit_params.n3)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.q1 = (selected_tree_1->fruit_params.q1 + selected_tree_2->fruit_params.q1)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.q2 = (selected_tree_1->fruit_params.q2 + selected_tree_2->fruit_params.q2)/randomFloatRange(1.5f, 2.5f);
        cross_fruit_params.q3 = (selected_tree_1->fruit_params.q3 + selected_tree_2->fruit_params.q3)/randomFloatRange(1.5f, 2.5f);

/*
        fruit_material->get_material()->diffuse = glm::vec3{randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.)};
        fruit_material->get_material()->sheen = randomFloatRange(0.2, 1.0);;
        fruit_material->get_material()->roughness = randomFloatRange(0.0001, 1.0);
        fruit_material->get_material()->clearcoat = 0.2f;
        fruit_material->get_material()->metallic = randomFloatRange(0.0001, 0.5);
        fruit_material->get_material()->emissive = randomFloatRange(0.0001, 1.) * fruit_material->get_material()->diffuse;
*/

        std::string leaf_mat = std::to_string(selected_tree_1->plantInfo.ID + selected_tree_2->plantInfo.ID).append("leaf_material");
        std::string fruit_mat = std::to_string(selected_tree_1->plantInfo.ID + selected_tree_2->plantInfo.ID).append("fruit_material");

        ev2::Ref<ev2::MaterialResource> new_fruit_material =  ResourceManager::get_singleton().get_material(fruit_mat);
        ev2::Ref<ev2::MaterialResource> new_leaf_material = ResourceManager::get_singleton().get_material(leaf_mat);
        
        new_fruit_material->get_material()->diffuse = (selected_tree_1->fruit_material->get_material()->diffuse + selected_tree_2->fruit_material->get_material()->diffuse)/randomFloatRange(1.5f, 2.5f);
        new_fruit_material->get_material()->emissive = (selected_tree_1->fruit_material->get_material()->emissive + selected_tree_2->fruit_material->get_material()->emissive)/randomFloatRange(1.5f, 2.5f);
        new_fruit_material->get_material()->metallic = (selected_tree_1->fruit_material->get_material()->metallic + selected_tree_2->fruit_material->get_material()->metallic)/randomFloatRange(1.5f, 2.5f);
        new_fruit_material->get_material()->subsurface = (selected_tree_1->fruit_material->get_material()->subsurface + selected_tree_2->fruit_material->get_material()->subsurface)/randomFloatRange(1.5f, 2.5f);
        new_fruit_material->get_material()->specular = (selected_tree_1->fruit_material->get_material()->specular + selected_tree_2->fruit_material->get_material()->specular)/randomFloatRange(1.5f, 2.5f);
        new_fruit_material->get_material()->roughness = (selected_tree_1->fruit_material->get_material()->roughness + selected_tree_2->fruit_material->get_material()->roughness)/randomFloatRange(1.5f, 2.5f);
        new_fruit_material->get_material()->specularTint = (selected_tree_1->fruit_material->get_material()->specularTint + selected_tree_2->fruit_material->get_material()->specularTint)/randomFloatRange(1.5f, 2.5f);
        new_fruit_material->get_material()->clearcoat = (selected_tree_1->fruit_material->get_material()->clearcoat + selected_tree_2->fruit_material->get_material()->clearcoat)/randomFloatRange(1.5f, 2.5f);
        new_fruit_material->get_material()->clearcoatGloss = (selected_tree_1->fruit_material->get_material()->clearcoatGloss + selected_tree_2->fruit_material->get_material()->clearcoatGloss)/randomFloatRange(1.5f, 2.5f);
        new_fruit_material->get_material()->sheen = (selected_tree_1->fruit_material->get_material()->sheen + selected_tree_2->fruit_material->get_material()->sheen)/randomFloatRange(1.5f, 2.5f);
        new_fruit_material->get_material()->sheenTint = (selected_tree_1->fruit_material->get_material()->sheenTint + selected_tree_2->fruit_material->get_material()->sheenTint)/randomFloatRange(1.5f, 2.5f);


        new_leaf_material->get_material()->diffuse = (selected_tree_1->leaf_material->get_material()->diffuse + selected_tree_2->leaf_material->get_material()->diffuse)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->emissive = (selected_tree_1->leaf_material->get_material()->emissive + selected_tree_2->leaf_material->get_material()->emissive)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->metallic = (selected_tree_1->leaf_material->get_material()->metallic + selected_tree_2->leaf_material->get_material()->metallic)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->subsurface = (selected_tree_1->leaf_material->get_material()->subsurface + selected_tree_2->leaf_material->get_material()->subsurface)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->specular = (selected_tree_1->leaf_material->get_material()->specular + selected_tree_2->leaf_material->get_material()->specular)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->roughness = (selected_tree_1->leaf_material->get_material()->roughness + selected_tree_2->leaf_material->get_material()->roughness)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->specularTint = (selected_tree_1->leaf_material->get_material()->specularTint + selected_tree_2->leaf_material->get_material()->specularTint)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->clearcoat = (selected_tree_1->leaf_material->get_material()->clearcoat + selected_tree_2->leaf_material->get_material()->clearcoat)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->clearcoatGloss = (selected_tree_1->leaf_material->get_material()->clearcoatGloss + selected_tree_2->leaf_material->get_material()->clearcoatGloss)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->sheen = (selected_tree_1->leaf_material->get_material()->sheen + selected_tree_2->leaf_material->get_material()->sheen)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->sheenTint = (selected_tree_1->leaf_material->get_material()->sheenTint + selected_tree_2->leaf_material->get_material()->sheenTint)/randomFloatRange(1.5f, 2.5f);
        new_leaf_material->get_material()->diffuse_tex = ResourceManager::get_singleton().get_texture("coffee_leaf1.png");

        glm::vec3 color_0 = glm::vec3(randomFloatRange(selected_tree_1->c0.r, selected_tree_2->c0.r) + randomFloatRange(-.2f, .2f), randomFloatRange(selected_tree_1->c0.g, selected_tree_2->c0.g) + randomFloatRange(-.2f, .2f), randomFloatRange(selected_tree_1->c0.b, selected_tree_2->c0.b) + randomFloatRange(-.2f, .2f)); 
        glm::vec3 color_1 = glm::vec3(randomFloatRange(selected_tree_1->c1.r, selected_tree_2->c1.r) + randomFloatRange(-.2f, .2f), randomFloatRange(selected_tree_1->c1.g, selected_tree_2->c1.g) + randomFloatRange(-.2f, .2f), randomFloatRange(selected_tree_1->c1.b, selected_tree_2->c1.b) + randomFloatRange(-.2f, .2f));
        float cross_leaf_scale = randomFloatRange(selected_tree_1->leaf_scale, selected_tree_2->leaf_scale) + randomFloatRange(-.2f, .2f);
        float cross_fruit_spawn_rate = (selected_tree_1->fruit_spawn_rate + selected_tree_2->fruit_spawn_rate)/randomFloatRange(1.5f, 2.5f);
        spawn_tree(position, rotation, crossed_params, (selected_tree_1->plantInfo.iterations + selected_tree_2->plantInfo.iterations)/2, color_0, color_1, 0.f, cross_leaf_scale, new_leaf_material, new_fruit_material, cross_fruit_spawn_rate, true);
    }
    //plantlist.push_back((Plant(unique_id, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), supershape, tree, tree_hit_sphere)));
}
