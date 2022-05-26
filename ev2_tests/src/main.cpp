
#include <iostream>
#include <filesystem>

#include <tests.h>
#include <tree.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
//#define DR_MP3_IMPLEMENTATION
//#include "../extras/dr_mp3.h"   /* Enables MP3 decoding. */

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h> 

#include <ev.h>
#include <ev_gl.h>
#include <window.h>
#include <shader.h>
#include <application.h>
#include <camera.h>
#include <window.h>
#include <vertex_buffer.h>
#include <resource.h>
#include <Sphere.h>
#include <physics.h>
#include <renderer.h>
#include <scene.h>
#include <physics.h>
#include <visual_nodes.h>
#include <debug.h>
#include <game.h>

namespace fs = std::filesystem;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

class TestApp : public ev2::Application {
public:
    TestApp() {}
    
    TestApp(const fs::path& asset_path) :   asset_path{asset_path} {}

    fs::path asset_path = fs::path("asset");

    ev2::Ref<ev2::CameraNode> cam_orbital{};
    ev2::Ref<ev2::Node> cam_orbital_root{};

    std::unique_ptr<GameState> game;

    glm::vec2 mouse_p{};
    glm::vec2 mouse_delta{};
    glm::vec2 move_input{};
    bool left_mouse_down = false;
    bool right_mouse_down = false;
    bool placeChild = false;
    float cam_x = 0, cam_y = 0;
    float cam_boom_length = 10.0f;
    int32_t window_width = 1920;
    int32_t window_height = 1080;

    bool show_debug = false;

    ev2::Ref<ev2::CameraNode> getCameraNode() {
        return cam_orbital;
    }

    void imgui(GLFWwindow * window) {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_debug) {
            show_material_editor_window();
            show_settings_editor_window();
            show_game_debug_window(game.get());
        }
        if (game->selected_tree_1->plantInfo.ID != -1) {
            ImGui::SetNextWindowSize(ImVec2(window_width/4, window_height/4));
            ImGui::SetNextWindowPos(ImVec2(window_width - window_width/4, 0));
            show_tree_window(game.get(), game->selected_tree_1);
        }
        if (game->selected_tree_2->plantInfo.ID != -1) {
            ImGui::SetNextWindowSize(ImVec2(window_width/4, window_height/4));
            ImGui::SetNextWindowPos(ImVec2(window_width - window_width/4, window_height/4));
            show_tree_window(game.get(), game->selected_tree_2);
        } 

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void initialize() {
        game = std::make_unique<GameState>();

        cam_orbital      = game->scene->create_node<ev2::CameraNode>("Orbital");
        cam_orbital_root = game->scene->create_node<ev2::Node>("cam_orbital_root");

        cam_orbital_root->add_child(cam_orbital);

        ev2::ResourceManager::get_singleton().loadGLTF(fs::path("models") / "Box.gltf");
       
    }


    int run() {
#if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        const char* glsl_version = "#version 100";
#elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        const char* glsl_version = "#version 150";
#else
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
#endif

        GLFWwindow* window = ev2::window::getContextWindow();
        if (window == NULL)
            return 1;
        glfwMakeContextCurrent(window);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        game->scene->ready();

        float dt = 0.05f;
        while(ev2::window::frame()) {
            //Passing io to manage focus between app behavior and imgui behavior on mouse events.
            update(dt, io);
            game->update(dt);
            ev2::Physics::get_singleton().simulate(dt);
            game->scene->update_pre_render();
            ev2::ResourceManager::get_singleton().pre_render();
            auto camera_node = getCameraNode();
            if (!show_debug)
                camera_node = game->cam_first_person;
            ev2::renderer::Renderer::get_singleton().render(camera_node->get_camera());
            imgui(window);
            dt = float(ev2::window::getFrameTime());
        }

        game->scene->destroy();
     
        return 0;
    }

    void toggleWireframe() {
        static bool enabled = false;
        enabled = !enabled;
        ev2::renderer::Renderer::get_singleton().set_wireframe(enabled);
    }


    void update(float dt, ImGuiIO& io) {

        if (show_debug && (right_mouse_down || ev2::window::getMouseCaptured()) && !io.WantCaptureMouse) {
            mouse_delta = ev2::window::getCursorPosition() - mouse_p;
            mouse_p = ev2::window::getCursorPosition();
            cam_x += mouse_delta.x * -.005f;
            cam_y = glm::clamp<float>(cam_y + mouse_delta.y * -.005f, glm::radians(-85.0f), glm::radians(85.0f));
        }

        glm::vec3 boom = {0, 0, cam_boom_length};
        glm::mat4 cam_t = glm::rotate(glm::mat4{1.0f}, (float)cam_y, glm::vec3{1, 0, 0});
        cam_t = glm::rotate(glm::mat4{1.0f}, (float)cam_x, {0, 1, 0}) * cam_t;

        boom = cam_t * glm::vec4(boom, 1.0f);

        cam_orbital->transform.position = boom;
        cam_orbital->transform.rotation = glm::quatLookAt(-glm::normalize(boom), glm::vec3{0, 1, 0});

        
        if (show_debug && glm::length(move_input) > 0.0f) {
            glm::vec2 input = glm::normalize(move_input);
            glm::vec3 cam_forward = glm::normalize(cam_orbital->get_camera().get_forward() * glm::vec3{1, 0, 1});
            glm::vec3 cam_right = glm::normalize(cam_orbital->get_camera().get_right() * glm::vec3{1, 0, 1});
            cam_orbital_root->transform.position +=
                cam_forward * 1.0f * cam_boom_length * dt * input.y + 
                cam_right * 1.0f * cam_boom_length * dt * input.x
            ; // camera movement on y plane
        }
    }

    void onKey(ev2::input::Key::Enum key, ev2::input::Modifier mods, bool down) override {
        ImGuiIO& io = ImGui::GetIO();
        switch (key) {
            case ev2::input::Key::Esc:
                if (down) {
                    show_debug = !show_debug;
                    ev2::window::setMouseCaptured(!show_debug);
                    ev2::input::SetInputEnabled(!show_debug);
                } 
                break;
            default:
                break;
        }
        if (!show_debug)
            ev2::input::SetKeyState(key, mods, down);
        if (show_debug && !io.WantCaptureMouse) {
            switch (key) {
                case ev2::input::Key::Tab:
                    break;
                case ev2::input::Key::KeyP:
                    break;
                case ev2::input::Key::KeyF:
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
        } else {
            move_input = glm::vec2{};
        }
    }

    void on_char(uint32_t scancode) override {}

    void on_scroll(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {
        auto& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            static int32_t scroll_last = scroll_pos;
            int32_t scroll_delta = scroll_pos - scroll_last;
            scroll_last = scroll_pos;
            cam_boom_length = glm::clamp(cam_boom_length - scroll_delta, 0.f, 200.f);
        }
    }

    void cursor_pos(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {
        if (!show_debug) {
            glm::vec2 scr_size = ev2::window::getWindowSize();
            glm::vec2 s_pos = ev2::window::getCursorPosition() / scr_size;
            ev2::input::SetMousePosition(s_pos);
        }
    }

    void on_mouse_button(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos, ev2::input::MouseButton::Enum button, bool down) override {
        mouse_p = ev2::window::getCursorPosition();
        if (button == 1)
            left_mouse_down = down;
        if (button == 3)
            right_mouse_down = down;
        ev2::input::SetMouseButton(button, down);
    }

    void on_window_size_change(int32_t width, int32_t height) override {
        if (ev2::renderer::Renderer::is_initialized())
            ev2::renderer::Renderer::get_singleton().set_resolution(width, height);
        window_width = width;
        window_height = height;
    }

    void on_drop_file(const std::string& path) override {}
};

//Callback for miniaudio.
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    /* Reading PCM frames will loop based on what we specified when called ma_data_source_set_looping(). */
    ma_data_source_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}

int initAudio(fs::path asset_path) {
    ma_result result;
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;
    const char* filePath = "";

    filePath = (asset_path / "stickerbrush.mp3").generic_string().c_str();
    if (filePath == "") {
        printf("No input file.\n");
        return -1;
    }
    printf(filePath);
    result = ma_decoder_init_file(filePath, NULL, &decoder);
    if (result != MA_SUCCESS) {
        return -2;
    }

    ma_data_source_set_looping(&decoder, MA_TRUE);

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -4;
    }
}

int main(int argc, char *argv[]) {


    ev2::Args args{argc, argv};

    fs::path asset_path = fs::path("asset");

    ev2::EV2_init(args, asset_path);
    ev2::window::setWindowTitle("Plant Game");

    std::unique_ptr<TestApp> app = std::make_unique<TestApp>(asset_path);
    ev2::window::setApplication(app.get());
    //initAudio(asset_path);
    app->initialize();

    int rv = app->run();

    // shutdown
    ev2::window::setApplication(nullptr);
    app = {};
    ev2::EV2_shutdown();
    return rv;
    //TODO: uninit audio device and decoder.
}