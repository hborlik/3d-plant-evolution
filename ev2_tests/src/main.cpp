
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

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

class TestApp : public ev2::Application {
public:
    ev2::Camera cam;

    fs::path asset_path = fs::path("asset");

    ev2::Program prog{"phong"};
    std::unique_ptr<ev2::Model> cube;

    uint32_t width = 800, height = 600;

    glm::vec2 mouse_p;
    glm::vec2 mouse_delta;
    bool mouse_down;
    float cam_x, cam_y;

    void load_shaders() {
        fs::path shader_dir = asset_path / "shader"; 
        prog.loadShader(ev2::gl::GLSLShaderType::VERTEX_SHADER, shader_dir/"phong.glsl.vert");
        prog.loadShader(ev2::gl::GLSLShaderType::FRAGMENT_SHADER, shader_dir/"phong.glsl.frag");
        prog.link();
        
        std::cout << prog;
    }

    void load_models() {
        cube = ev2::loadObj("cube.obj", asset_path / "models");
    }

    int run() {
        cam.setPosition({0, 0, 2});

        glClearColor(.72f, .84f, 1.06f, 1.0f);
        // Enable z-buffer test.
        glEnable(GL_DEPTH_TEST);

        while(ev2::window::frame()) {
            float dt = float(ev2::window::getFrameTime());

            render(dt);
        }
        return 0;
    }

    void render(float dt) {
        // first update scene

        if (mouse_down) {
            glm::vec2 m = ev2::window::getCursorPosition();
            mouse_delta = m - mouse_p;
            mouse_p = m;
        } else {
            mouse_delta = {};
            mouse_p = ev2::window::getCursorPosition();
        }

        cam_x += mouse_delta.x * -150.0f * dt;
        cam_y += mouse_delta.y * -150.0f * dt;

        glm::vec3 boom = {0, 0, 3};
        glm::mat4 cam_t = glm::rotate(glm::mat4{1.0f}, (float)cam_y, glm::vec3{1, 0, 0});
        cam_t = glm::rotate(glm::mat4{1.0f}, (float)cam_x, {0, 1, 0}) * cam_t;

        boom = cam_t * glm::vec4(boom, 1.0f);

        cam.setPosition(boom);
        cam.setRotation(glm::quatLookAt(-glm::normalize(boom), glm::vec3{0, 1, 0}));

        // render scene
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        prog.use();

        prog.setShaderParameter("P", cam.getProjection());
        prog.setShaderParameter("V", cam.getView());
        prog.setShaderParameter("CameraPos", cam.getPosition());

        glm::mat4 M = glm::identity<glm::mat4>();//glm::rotate(glm::identity<glm::mat4>(), 0.3f, glm::vec3{0, 1, 0});
        glm::mat3 G = glm::inverse(glm::transpose(glm::mat3(M)));
        ev2::gl::glUniform(M, prog.getUniformInfo("M").Location);
        ev2::gl::glUniform(G, prog.getUniformInfo("G").Location);
        cube->draw();
    }

    void onKey(ev2::input::Key::Enum key, ev2::input::Modifier mods, bool down) override {}

    void onChar(uint32_t scancode) override {}

    void onScroll(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {}

    void cursorPos(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {}

    void onMouseButton(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos, ev2::input::MouseButton::Enum button, bool down) override {
        mouse_down = down;
    }

    void onWindowSizeChange(int32_t width, int32_t height) override {
        this->width = width;
        this->height = height;
        float aspect = width/(float)height;
        cam.setProjection(glm::perspective(glm::radians(50.0f), aspect, 0.1f, 200.0f));
    }

    void onDropFile(const std::string& path) override {}
};

static std::unique_ptr<TestApp> app;


int main(int argc, char *argv[]) {
    ev2::Args args{argc, argv};

    ev2::EV2_init(args);
    ev2::window::setWindowTitle("Testing");

    app = std::make_unique<TestApp>();
    ev2::window::setApplication(app.get());

    app->load_shaders();
    app->load_models();

    return app->run();;
}