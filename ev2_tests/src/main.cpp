
#include <tests.h>

#include <ev.h>
#include <window.h>
#include <shader.h>
#include <application.h>
#include <camera.h>
#include <window.h>
#include <mesh.h>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

class TestApp : public ev2::Application {
public:
    ev2::Camera cam;

    fs::path asset_path = fs::path("asset");

    ev2::Program prog{"basic"};
    std::unique_ptr<ev2::Model> cube;

    void load_shaders() {
        fs::path shader_dir = asset_path / "shader"; 
        prog.loadShader(ev2::gl::GLSLShaderType::FRAGMENT_SHADER, shader_dir/"basic.glsl.frag");
        prog.loadShader(ev2::gl::GLSLShaderType::VERTEX_SHADER, shader_dir/"basic.glsl.vert");
        prog.link();
        
        std::cout << prog;
    }

    void load_models() {

    }

    int run() {
        while(ev2::window::frame()) {
            float dt = float(ev2::window::getFrameTime());

            render(dt);
        }
    }

    void render(float dt) {

    }

    void onKey(ev2::input::Key::Enum key, ev2::input::Modifier mods, bool down) override {}

    void onChar(uint32_t scancode) override {}

    void onScroll(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {}

    void cursorPos(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {}

    void onMouseButton(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos, ev2::input::MouseButton::Enum button, bool down) override {}

    void onWindowSizeChange(int32_t width, int32_t height) override {}

    void onDropFile(const std::string& path) override {}
};

static std::unique_ptr<TestApp> app;


int main(int argc, char *argv[]) {
    ev2::Args args{argc, argv};

    ev2::EV2_init(args);
    ev2::window::setWindowTitle("Testing");

    app = std::make_unique<TestApp>();

    app->load_shaders();

    return app->run();;
}