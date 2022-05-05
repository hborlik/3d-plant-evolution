
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

    ev2::Camera cam_orbital{};
    ev2::Camera cam_fly{};
    ev2::Camera cam_first_person{};

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

    uint32_t width = 800, height = 600;

    ev2::Camera& getActiveCam() {
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

        // ground_cube->materials[0].diffuse = {0.1, 0.6, 0.05};
        // ground_cube->materials[0].shininess = 0.02;

        auto h_node = scene->create_node<ev2::VisualInstance>("house");
        h_node->set_model(hid);
        h_node->transform.position -= glm::vec3{0, 5, 0};
        auto g_node = scene->create_node<ev2::VisualInstance>("ground");
        g_node->set_model(ground);
        g_node->set_material_override(1);
        g_node->transform.scale = glm::vec3{1000, 0.1, 1000};
    }

    void updateShape(float dt, Sphere geometry) {
        // cube = geometry.getModel();
    }


    int run() {
        float dt = 0.05f;
        while(ev2::window::frame()) {
            update(dt);
            ev2::Renderer::get_singleton().render(getActiveCam());
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

        cam_orbital.set_position(boom);
        cam_orbital.set_rotation(glm::quatLookAt(-glm::normalize(boom), glm::vec3{0, 1, 0}));

        cam_first_person.set_rotation(glm::rotate(glm::rotate(glm::identity<glm::quat>(), (float)cam_x, glm::vec3{0, 1, 0}), (float)cam_y, glm::vec3{1, 0, 0}));
        if (camera_type == FirstPerson && glm::length(move_input) > 0.0f) {
            glm::vec2 input = glm::normalize(move_input);
            glm::vec3 cam_forward = glm::normalize(cam_first_person.get_forward() * glm::vec3{1, 0, 1});
            glm::vec3 cam_right = glm::normalize(cam_first_person.get_right() * glm::vec3{1, 0, 1});
            cam_first_person.set_position(
                cam_first_person.get_position() * glm::vec3{1, 0, 1} + 
                glm::vec3{0, 2, 0} + 
                cam_forward * 10.0f * dt * input.y + 
                cam_right * 10.0f * dt * input.x
            ); // force camera movement on y plane
        }

        // render scene

        

        // glViewport(0, 0, width, height);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // prog.use();

        // prog.setShaderParameter("P", getActiveCam().get_projection());
        // prog.setShaderParameter("V", getActiveCam().get_view());
        // prog.setShaderParameter("CameraPos", getActiveCam().get_position());

        // glm::mat4 M = glm::translate(glm::identity<glm::mat4>(), glm::vec3{0, 3, -5}) * glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{0, 0, 1});
        // glm::mat3 G = glm::inverse(glm::transpose(glm::mat3(M)));
        // //glm::mat4 Trans = glm::translate( glm::mat4(1.0f), glm::vec3(0, -1, -1));
        // //glm::mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
        // //glm::mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
        // glm::mat4 ScaleS = glm::scale(glm::mat4(1.0f), glm::vec3(0.1, 0.1, 0.1));        
        // M = M*ScaleS;
        // ev2::gl::glUniform(M, prog.getUniformInfo("M").Location);
        // ev2::gl::glUniform(G, prog.getUniformInfo("G").Location);

        // cube->draw(prog);

        // for (auto& p : plants) {
        //     updateShape(dt, p.geometry);
        //     M = glm::translate(glm::identity<glm::mat4>(), p.position) * glm::mat4_cast(p.rot);
        //     G = glm::inverse(glm::transpose(glm::mat3(M)));
        //     glm::mat4 ScaleS = glm::scale(glm::mat4(1.0f), glm::vec3(0.1, 0.1, 0.1));        
        //     M = M*ScaleS;            
        //     ev2::gl::glUniform(M, prog.getUniformInfo("M").Location);
        //     ev2::gl::glUniform(G, prog.getUniformInfo("G").Location);
        //     cube->materials[0].diffuse = p.color;
        //     cube->materials[0].ambient = p.color;
        //     cube->materials[0].emission = p.color * 0.9f;
            
        //     cube->draw(prog);
        // }

        // // house
        // M = glm::translate(glm::identity<glm::mat4>(), {40, -17, 40});
        // M = M * glm::scale(glm::identity<glm::mat4>(), glm::vec3(2, 2, 2));
        // G = glm::inverse(glm::transpose(glm::mat3(M)));

        // ev2::gl::glUniform(M, prog.getUniformInfo("M").Location);
        // ev2::gl::glUniform(G, prog.getUniformInfo("G").Location);
        // house->draw(prog);

        // // ground cube
        // M = glm::translate(glm::identity<glm::mat4>(), {0, -5, 0}) * glm::scale(glm::identity<glm::mat4>(), {300, 0.2, 300});
        // G = glm::inverse(glm::transpose(glm::mat3(M)));
        
        // ev2::gl::glUniform(M, prog.getUniformInfo("M").Location);
        // ev2::gl::glUniform(G, prog.getUniformInfo("G").Location);
        // ground_cube->draw(prog);
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
        this->width = width;
        this->height = height;
        float aspect = width/(float)height;
        auto p = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 200.0f);
        cam_orbital.set_projection(p);
        cam_fly.set_projection(p);
        cam_first_person.set_projection(p);
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