
#include <iostream>
#include <filesystem>

#include <tests.h>
#include <tree.h>

#include <ev.h>
#include <ev_gl.h>
#include <window.h>
#include <shader.h>
#include <application.h>
#include <camera.h>
#include <window.h>
#include <mesh.h>
#include <resource.h>
#include <Sphere.h>
#include <renderer.h>
#include <scene.h>
#include <visual_nodes.h>


namespace fs = std::filesystem;

float randomFloatTo(float limit) {
    return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/limit));
}

struct Plant {
    glm::vec3 position;
    glm::vec3 color;
    glm::quat rot;
    Sphere geometry;
    Plant(glm::vec3 pos, glm::vec3 col, Sphere geo) {position = pos; color = col; geometry = geo; 
        rot = glm::quatLookAt(glm::vec3(randomFloatTo(2) - 1, 0, randomFloatTo(2) - 1), glm::vec3{0, 1, 0});
    }
};

class TestApp : public ev2::Application {
public:
    TestApp() : RM{std::make_unique<ev2::ResourceManager>(asset_path)},
                scene{std::make_unique<ev2::Scene>(RM)} {}
    
    TestApp(const fs::path& asset_path) :   asset_path{asset_path}, 
                                            RM{std::make_unique<ev2::ResourceManager>(asset_path)}, 
                                            scene{std::make_unique<ev2::Scene>(RM)} {}

    fs::path asset_path = fs::path("asset");

    ev2::Ref<ev2::CameraNode> cam_orbital{};
    ev2::Ref<ev2::CameraNode> cam_fly{};
    ev2::Ref<ev2::CameraNode> cam_first_person{};

    ev2::Ref<ev2::Node> tree{};

    std::shared_ptr<ev2::ResourceManager> RM;
    std::unique_ptr<ev2::Scene> scene;

    glm::vec2 mouse_p{};
    glm::vec2 mouse_delta{};
    glm::vec2 move_input{};
    bool mouse_down = false;
    float cam_x = 0, cam_y = 0;

    enum CameraMode : uint8_t {
        FirstPerson = 0,
        Fly,
        Orbital
    } camera_type;

    ev2::Ref<ev2::CameraNode> getActiveCam() {
        switch(camera_type) {
            case Orbital:
                return cam_orbital;
            case FirstPerson:
                return cam_first_person;
            case Fly:
            default:
                return cam_fly;
        }
    }


    void initialize() {

        Sphere supershape(1.0f , 20, 20);
        // cube =  supershape.getModel();


        ev2::MID hid = RM->get_model( fs::path("models") / "rungholt" / "house.obj");
        ev2::MID ground = RM->get_model( fs::path("models") / "cube.obj");
        ev2::MID building0 = RM->get_model( fs::path("models") / "low_poly_houses.obj");

        // ground_cube->materials[0].diffuse = {0.1, 0.6, 0.05};
        // ground_cube->materials[0].shininess = 0.02;

        auto light = scene->create_node<ev2::DirectionalLightNode>("directional_light");
        light->transform.position = glm::vec3{10, 100, 0};

        auto h_node = scene->create_node<ev2::VisualInstance>("house");
        h_node->set_model(hid);
        h_node->transform.position -= glm::vec3{0, 5, 0};

        auto lh_node = scene->create_node<ev2::VisualInstance>("building");
        lh_node->transform.position = glm::vec3{50, 0, 50};
        lh_node->set_model(building0);

        auto g_node = scene->create_node<ev2::VisualInstance>("ground");
        g_node->set_model(ground);
        g_node->set_material_override(1);
        g_node->transform.scale = glm::vec3{1000, 0.1, 1000};
        g_node->transform.position = glm::vec3{0, 0.4, 0};

        cam_orbital      = scene->create_node<ev2::CameraNode>("Orbital");
        cam_fly          = scene->create_node<ev2::CameraNode>("Fly");
        cam_first_person = scene->create_node<ev2::CameraNode>("FP");

        tree = scene->create_node<TreeNode>("Tree");
        tree->transform.position = glm::vec3{-50, 0, 0};
    }

    void updateShape(float dt, Sphere geometry) {
        // cube = geometry.getModel();
    }


    int run() {
        float dt = 0.05f;
        while(ev2::window::frame()) {
            update(dt);
            ev2::Renderer::get_singleton().render(getActiveCam()->get_camera());
            dt = float(ev2::window::getFrameTime());
        }
        ev2::Renderer::shutdown();
        return 0;
    }

    void toggleWireframe() {
        static bool enabled = false;
        enabled = !enabled;
        ev2::Renderer::get_singleton().set_wireframe(enabled);
    }

    void update(float dt) {
        // first update scene
        scene->update(dt);

        if (mouse_down || ev2::window::getMouseCaptured()) {
            mouse_delta = ev2::window::getCursorPosition() - mouse_p;
            mouse_p = ev2::window::getCursorPosition();
            cam_x += mouse_delta.x * -.005f;
            cam_y = glm::clamp<float>(cam_y + mouse_delta.y * -.005f, glm::radians(-85.0f), glm::radians(85.0f));
        }

        glm::vec3 boom = {0, 0, 70};
        glm::mat4 cam_t = glm::rotate(glm::mat4{1.0f}, (float)cam_y, glm::vec3{1, 0, 0});
        cam_t = glm::rotate(glm::mat4{1.0f}, (float)cam_x, {0, 1, 0}) * cam_t;

        boom = cam_t * glm::vec4(boom, 1.0f);

        cam_orbital->transform.position = boom;
        cam_orbital->transform.rotation = glm::quatLookAt(-glm::normalize(boom), glm::vec3{0, 1, 0});

        cam_first_person->transform.rotation = glm::rotate(glm::rotate(glm::identity<glm::quat>(), (float)cam_x, glm::vec3{0, 1, 0}), (float)cam_y, glm::vec3{1, 0, 0});
        if (camera_type == FirstPerson && glm::length(move_input) > 0.0f) {
            glm::vec2 input = glm::normalize(move_input);
            glm::vec3 cam_forward = glm::normalize(cam_first_person->get_camera().get_forward() * glm::vec3{1, 0, 1});
            glm::vec3 cam_right = glm::normalize(cam_first_person->get_camera().get_right() * glm::vec3{1, 0, 1});
            cam_first_person->transform.position = glm::vec3(
                cam_first_person->transform.position * glm::vec3{1, 0, 1} + 
                glm::vec3{0, 2, 0} + 
                cam_forward * 10.0f * dt * input.y + 
                cam_right * 10.0f * dt * input.x
            ); // force camera movement on y plane
        }
    }

    void onKey(ev2::input::Key::Enum key, ev2::input::Modifier mods, bool down) override {
        switch (key) {
            case ev2::input::Key::Tab:
                if (down)
                    ev2::window::setMouseCaptured(!ev2::window::getMouseCaptured());
                break;
            case ev2::input::Key::KeyP:
                break;
            case ev2::input::Key::Esc:
                break;
            case ev2::input::Key::KeyF:
                if (down)
                    camera_type = CameraMode((camera_type + 1) % 3);
                break;
            case ev2::input::Key::KeyZ:
                if (down)
                    toggleWireframe();
                break;
            case ev2::input::Key::KeyW:
                move_input.y = down ? 1.0f : 0.0f;
                break;
            case ev2::input::Key::KeyS:
                move_input.y = down ? -1.0f : 0.0f;
                break;
            case ev2::input::Key::KeyA:
                move_input.x = down ? -1.0f : 0.0f;
                break;
            case ev2::input::Key::KeyD:
                move_input.x = down ? 1.0f : 0.0f;
                break;
            default:
                break;
        }
    }

    void onChar(uint32_t scancode) override {}

    void onScroll(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {}

    void cursorPos(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {}

    void onMouseButton(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos, ev2::input::MouseButton::Enum button, bool down) override {
        mouse_down = down;
        mouse_p = ev2::window::getCursorPosition();
    }

    void onWindowSizeChange(int32_t width, int32_t height) override {
        if (ev2::Renderer::is_initialized())
            ev2::Renderer::get_singleton().set_resolution(width, height);
    }

    void onDropFile(const std::string& path) override {}
};

int main(int argc, char *argv[]) {
    ev2::Args args{argc, argv};

    fs::path asset_path = fs::path("asset");

    ev2::EV2_init(args, asset_path);
    ev2::window::setWindowTitle("Plant Game");

    std::unique_ptr<TestApp> app = std::make_unique<TestApp>(asset_path);
    ev2::window::setApplication(app.get());

    app->initialize();

    return app->run();;
}