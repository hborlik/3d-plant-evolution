
#include <iostream>
#include <filesystem>

#include <tests.h>

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
#include <node.h>


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
    ev2::Camera cam_orbital;
    ev2::Camera cam_fly;
    ev2::Camera cam_first_person;

    ev2::Scene scene;

    glm::vec2 mouse_p;
    glm::vec2 mouse_delta;
    glm::vec2 move_input;
    bool mouse_down;
    float cam_x, cam_y;

    enum CameraMode : uint8_t {
        FirstPerson = 0,
        Fly,
        Orbital
    } camera_type;


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

    fs::path asset_path = fs::path("asset");

    ev2::Program prog{"phong"};
    std::shared_ptr<ev2::Model> cube;
    std::shared_ptr<ev2::Model> house;
    std::shared_ptr<ev2::Model> ground_cube;

    uint32_t width = 800, height = 600;

    void load_shaders() {
        fs::path shader_dir = asset_path / "shader"; 
        prog.loadShader(ev2::gl::GLSLShaderType::VERTEX_SHADER, shader_dir/"phong.glsl.vert");
        prog.loadShader(ev2::gl::GLSLShaderType::FRAGMENT_SHADER, shader_dir/"phong.glsl.frag");
        prog.link();
        
        std::cout << prog;

        ev2::Renderer::intitalize(width, height, shader_dir);

    }

    void load_models() {
        Sphere supershape(1.0f , 20, 20);
        cube =  supershape.getModel();
        house = ev2::loadObj("house.obj", asset_path / "models" / "rungholt");
        ground_cube = ev2::loadObj("cube.obj", asset_path / "models");
        ground_cube->materials[0].diffuse = {0.1, 0.6, 0.05};
        ground_cube->materials[0].shininess = 0.02;

        ev2::Renderer::get_singleton().create_model(1, cube);
        ev2::Renderer::get_singleton().create_model(2, house);
        ev2::Renderer::get_singleton().create_model(3, ground_cube);

        ev2::Renderer::get_singleton().create_instance(2);
    }

    void updateShape(float dt, Sphere geometry) {
        cube = geometry.getModel();
                //ev2::loadObj("house.obj", asset_path / "models" / "rungholt");
    }


    int run() {
        glClearColor(.72f, .84f, 1.06f, 1.0f);
        // Enable z-buffer test.
        glEnable(GL_DEPTH_TEST);

        float dt = 0.05f;
        while(ev2::window::frame()) {
            render(dt);
            dt = float(ev2::window::getFrameTime());
        }
        return 0;
    }

    void toggleWireframe() {
        static bool enabled = false;
        if (enabled)
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        else
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        enabled = !enabled;
    }

    void render(float dt) {
        // first update scene

        

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

        cam_orbital.setPosition(boom);
        cam_orbital.setRotation(glm::quatLookAt(-glm::normalize(boom), glm::vec3{0, 1, 0}));

        cam_first_person.setRotation(glm::rotate(glm::rotate(glm::identity<glm::quat>(), (float)cam_x, glm::vec3{0, 1, 0}), (float)cam_y, glm::vec3{1, 0, 0}));
        if (camera_type == FirstPerson && glm::length(move_input) > 0.0f) {
            glm::vec2 input = glm::normalize(move_input);
            glm::vec3 cam_forward = glm::normalize(cam_first_person.getForward() * glm::vec3{1, 0, 1});
            glm::vec3 cam_right = glm::normalize(cam_first_person.getRight() * glm::vec3{1, 0, 1});
            cam_first_person.setPosition(
                cam_first_person.getPosition() * glm::vec3{1, 0, 1} + 
                glm::vec3{0, 2, 0} + 
                cam_forward * 10.0f * dt * input.y + 
                cam_right * 10.0f * dt * input.x
            ); // force camera movement on y plane
        }

        // render scene

        ev2::Renderer::get_singleton().render(getActiveCam());

        // glViewport(0, 0, width, height);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // prog.use();

        // prog.setShaderParameter("P", getActiveCam().getProjection());
        // prog.setShaderParameter("V", getActiveCam().getView());
        // prog.setShaderParameter("CameraPos", getActiveCam().getPosition());

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
        cam_orbital.setProjection(p);
        cam_fly.setProjection(p);
        cam_first_person.setProjection(p);
    }

    void onDropFile(const std::string& path) override {}
};

int main(int argc, char *argv[]) {
    ev2::Args args{argc, argv};

    ev2::EV2_init(args);
    ev2::window::setWindowTitle("Plant Game");

    std::unique_ptr<TestApp> app = std::make_unique<TestApp>();
    ev2::window::setApplication(app.get());

    app->load_shaders();
    app->load_models();

    return app->run();;
}