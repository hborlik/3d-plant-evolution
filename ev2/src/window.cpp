/**
 * @file window.cpp
 * @version 0.1
 * @date 2019-09-04
 * 
 * 
 */
#include <window.h>

#include <iostream>
#include <iomanip>
#include <array>

#include <ev.h>
#include <input.h>
#include <application.h>

using namespace ev2;

namespace {

// glfw error callback
void glfw_error_callback(int error, const char *description)
{
	std::cerr << "GLFW ERROR: " << description << std::endl;
}

// gl error callback
__attribute__ ((stdcall))
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar *message, const void *userParam) {
    std::string output_severity{};
    std::string output_source{};
    std::string output_type{};
    std::string output_message{message};

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        output_severity = "NOTIFICATION";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        output_severity = "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        output_severity = "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        output_severity = "HIGH";
        break;
    default:
        break;
    }

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        output_source = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        output_source = "WINDOW_SYSTEM";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        output_source = "THIRD_PARTY";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        output_source = "APPLICATION";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        output_source = "OTHER";
        break;
    default:
        break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        output_type = "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        output_type = "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        output_type = "UNDEFINED_BEHAVIOR";
        break;
    //case GL_DEBUG_TYPE_PORTABILITIY:
    //    break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        output_type = "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_MARKER:
        output_type = "MARKER";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        output_type = "PUSH_GROUP";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        output_type = "POP_GROUP";
        break;
    case GL_DEBUG_TYPE_OTHER:
        output_type = "OTHER";
        break;
    default:
        break;
    }

    std::cout << "[" << std::setprecision(5) << std::fixed << glfwGetTime() << "] OpenGL[" << output_source << ":" << output_severity << ":" << output_type << "] " << output_message << std::endl;
}


input::Modifier translateKeyModifiers(int glfw_modifier)
{
    uint8_t modifiers = 0;
    if (glfw_modifier & GLFW_MOD_ALT)
        modifiers |= input::Modifier::LeftAlt;

    if (glfw_modifier & GLFW_MOD_CONTROL)
        modifiers |= input::Modifier::LeftCtrl;

    if (glfw_modifier & GLFW_MOD_SUPER)
        modifiers |= input::Modifier::LeftMeta;

    if (glfw_modifier & GLFW_MOD_SHIFT)
        modifiers |= input::Modifier::LeftShift;
    
    return {modifiers};
}

// key translations map
std::array<input::Key::Enum, GLFW_KEY_LAST + 1> s_translateKey;

input::Key::Enum translateKey(int _key)
{
    return s_translateKey[_key];
}

input::MouseButton::Enum translateMouseButton(int _button)
{
    if (_button == GLFW_MOUSE_BUTTON_LEFT) {
        return input::MouseButton::Left;
    }
    else if (_button == GLFW_MOUSE_BUTTON_RIGHT) {
        return input::MouseButton::Right;
    }
    return input::MouseButton::Middle;
}

} // namespace


class Context {
public:
    Context() {
        s_translateKey.fill(input::Key::Empty);
        s_translateKey[GLFW_KEY_ESCAPE]       = input::Key::Esc;
        s_translateKey[GLFW_KEY_ENTER]        = input::Key::Return;
        s_translateKey[GLFW_KEY_TAB]          = input::Key::Tab;
        s_translateKey[GLFW_KEY_BACKSPACE]    = input::Key::Backspace;
        s_translateKey[GLFW_KEY_SPACE]        = input::Key::Space;
        s_translateKey[GLFW_KEY_UP]           = input::Key::Up;
        s_translateKey[GLFW_KEY_DOWN]         = input::Key::Down;
        s_translateKey[GLFW_KEY_LEFT]         = input::Key::Left;
        s_translateKey[GLFW_KEY_RIGHT]        = input::Key::Right;
        s_translateKey[GLFW_KEY_PAGE_UP]      = input::Key::PageUp;
        s_translateKey[GLFW_KEY_PAGE_DOWN]    = input::Key::PageDown;
        s_translateKey[GLFW_KEY_HOME]         = input::Key::Home;
        s_translateKey[GLFW_KEY_END]          = input::Key::End;
        s_translateKey[GLFW_KEY_PRINT_SCREEN] = input::Key::Print;
        s_translateKey[GLFW_KEY_KP_ADD]       = input::Key::Plus;
        s_translateKey[GLFW_KEY_EQUAL]        = input::Key::Plus;
        s_translateKey[GLFW_KEY_KP_SUBTRACT]  = input::Key::Minus;
        s_translateKey[GLFW_KEY_MINUS]        = input::Key::Minus;
        s_translateKey[GLFW_KEY_COMMA]        = input::Key::Comma;
        s_translateKey[GLFW_KEY_PERIOD]       = input::Key::Period;
        s_translateKey[GLFW_KEY_SLASH]        = input::Key::Slash;
        s_translateKey[GLFW_KEY_F1]           = input::Key::F1;
        s_translateKey[GLFW_KEY_F2]           = input::Key::F2;
        s_translateKey[GLFW_KEY_F3]           = input::Key::F3;
        s_translateKey[GLFW_KEY_F4]           = input::Key::F4;
        s_translateKey[GLFW_KEY_F5]           = input::Key::F5;
        s_translateKey[GLFW_KEY_F6]           = input::Key::F6;
        s_translateKey[GLFW_KEY_F7]           = input::Key::F7;
        s_translateKey[GLFW_KEY_F8]           = input::Key::F8;
        s_translateKey[GLFW_KEY_F9]           = input::Key::F9;
        s_translateKey[GLFW_KEY_F10]          = input::Key::F10;
        s_translateKey[GLFW_KEY_F11]          = input::Key::F11;
        s_translateKey[GLFW_KEY_F12]          = input::Key::F12;
        s_translateKey[GLFW_KEY_KP_0]         = input::Key::NumPad0;
        s_translateKey[GLFW_KEY_KP_1]         = input::Key::NumPad1;
        s_translateKey[GLFW_KEY_KP_2]         = input::Key::NumPad2;
        s_translateKey[GLFW_KEY_KP_3]         = input::Key::NumPad3;
        s_translateKey[GLFW_KEY_KP_4]         = input::Key::NumPad4;
        s_translateKey[GLFW_KEY_KP_5]         = input::Key::NumPad5;
        s_translateKey[GLFW_KEY_KP_6]         = input::Key::NumPad6;
        s_translateKey[GLFW_KEY_KP_7]         = input::Key::NumPad7;
        s_translateKey[GLFW_KEY_KP_8]         = input::Key::NumPad8;
        s_translateKey[GLFW_KEY_KP_9]         = input::Key::NumPad9;
        s_translateKey[GLFW_KEY_0]            = input::Key::Key0;
        s_translateKey[GLFW_KEY_1]            = input::Key::Key1;
        s_translateKey[GLFW_KEY_2]            = input::Key::Key2;
        s_translateKey[GLFW_KEY_3]            = input::Key::Key3;
        s_translateKey[GLFW_KEY_4]            = input::Key::Key4;
        s_translateKey[GLFW_KEY_5]            = input::Key::Key5;
        s_translateKey[GLFW_KEY_6]            = input::Key::Key6;
        s_translateKey[GLFW_KEY_7]            = input::Key::Key7;
        s_translateKey[GLFW_KEY_8]            = input::Key::Key8;
        s_translateKey[GLFW_KEY_9]            = input::Key::Key9;
        s_translateKey[GLFW_KEY_A]            = input::Key::KeyA;
        s_translateKey[GLFW_KEY_B]            = input::Key::KeyB;
        s_translateKey[GLFW_KEY_C]            = input::Key::KeyC;
        s_translateKey[GLFW_KEY_D]            = input::Key::KeyD;
        s_translateKey[GLFW_KEY_E]            = input::Key::KeyE;
        s_translateKey[GLFW_KEY_F]            = input::Key::KeyF;
        s_translateKey[GLFW_KEY_G]            = input::Key::KeyG;
        s_translateKey[GLFW_KEY_H]            = input::Key::KeyH;
        s_translateKey[GLFW_KEY_I]            = input::Key::KeyI;
        s_translateKey[GLFW_KEY_J]            = input::Key::KeyJ;
        s_translateKey[GLFW_KEY_K]            = input::Key::KeyK;
        s_translateKey[GLFW_KEY_L]            = input::Key::KeyL;
        s_translateKey[GLFW_KEY_M]            = input::Key::KeyM;
        s_translateKey[GLFW_KEY_N]            = input::Key::KeyN;
        s_translateKey[GLFW_KEY_O]            = input::Key::KeyO;
        s_translateKey[GLFW_KEY_P]            = input::Key::KeyP;
        s_translateKey[GLFW_KEY_Q]            = input::Key::KeyQ;
        s_translateKey[GLFW_KEY_R]            = input::Key::KeyR;
        s_translateKey[GLFW_KEY_S]            = input::Key::KeyS;
        s_translateKey[GLFW_KEY_T]            = input::Key::KeyT;
        s_translateKey[GLFW_KEY_U]            = input::Key::KeyU;
        s_translateKey[GLFW_KEY_V]            = input::Key::KeyV;
        s_translateKey[GLFW_KEY_W]            = input::Key::KeyW;
        s_translateKey[GLFW_KEY_X]            = input::Key::KeyX;
        s_translateKey[GLFW_KEY_Y]            = input::Key::KeyY;
        s_translateKey[GLFW_KEY_Z]            = input::Key::KeyZ;
    }

    ~Context() {
        if(window_ptr)
            glfwDestroyWindow(window_ptr);
        glfwTerminate();
    }

    void init() {
        glfwSetErrorCallback(glfw_error_callback);

        if(glfwInit() != GLFW_TRUE) {
            throw engine_exception{"Cannot initialize GLFW"};
        }

        // system info
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        int width_mm, height_mm, defaultWidth, defaultHeight;
        float scale;
        glfwGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);
        glfwGetMonitorContentScale(monitor, &scale, nullptr);
        glfwGetMonitorWorkarea(monitor, nullptr, nullptr, &defaultWidth, &defaultHeight);
        if (width_mm == 0 || height_mm == 0) {
            std::clog << "glfwGetMonitorPhysicalSize failed!" << std::endl;
            width_mm = defaultWidth;
            height_mm = defaultHeight;
        }

        //request the highest possible version of OpenGL
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // for debugging in 4.3 and later
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        window_ptr = glfwCreateWindow(defaultWidth
            , defaultHeight
            , "window"
            , NULL
            , NULL
            );

        if (!window_ptr)
        {
            std::clog << "glfwCreateWindow failed!" << std::endl;
            glfwTerminate();
            throw engine_exception{"Cannot glfwCreateWindow"};
        }

        glfwSetKeyCallback(window_ptr, keyCb);
        glfwSetCharCallback(window_ptr, charCb);
        glfwSetScrollCallback(window_ptr, scrollCb);
        glfwSetCursorPosCallback(window_ptr, cursorPosCb);
        glfwSetMouseButtonCallback(window_ptr, mouseButtonCb);
        glfwSetWindowSizeCallback(window_ptr, windowSizeCb);
        glfwSetDropCallback(window_ptr, dropFileCb);

        // make the given window context the current glfw context. required for below
        glfwMakeContextCurrent(window_ptr);

        // load gl functions
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::clog << "Failed to initialize GLAD" << std::endl;
            throw engine_exception{"Failed to initialize GLAD"};
        }

        if(glfwRawMouseMotionSupported())
            glfwSetInputMode(window_ptr, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

        // Set vsync interval
        glfwSwapInterval(1);

        // only for core version 4.3 and later
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(gl_debug_callback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

        const GLubyte *renderer = glGetString( GL_RENDERER );
        const GLubyte *vendor = glGetString( GL_VENDOR );
        const GLubyte *version = glGetString( GL_VERSION );
        const GLubyte *glslVersion = glGetString( GL_SHADING_LANGUAGE_VERSION );

        GLint major, minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        
        printf("GL: %s %s\n", vendor, renderer);
        printf("GL Version: %s (%d.%d)\n", version, major, minor);
        printf("GLSL Version : %s\n", glslVersion);

        // push a test message
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, 25, "TEST DEBUG MESSAGE");
    }

    bool frame() {
        // display frame
        glfwSwapBuffers(window_ptr);
        // update inputs
        glfwPollEvents();
        // mark this frame time
        lastTime = glfwGetTime();

        return !glfwWindowShouldClose(window_ptr);
    }

    int run() {
        lastTime = glfwGetTime();
        while(!glfwWindowShouldClose(window_ptr)) {
            double deltaTime = getDelta();
            // render
            // renderer->render(deltaTime);
            lastTime = glfwGetTime();

            // display frame
            glfwSwapBuffers(window_ptr);
            // update inputs
            glfwPollEvents();
        }

        return 0;
    }

    double getDelta() {
        return glfwGetTime() - lastTime;
    }

    void setWindowTitle(const std::string& title) {
        glfwSetWindowTitle(window_ptr, title.data());
    }

    glm::vec2 getCursor() const {
        double x, y;
        glfwGetCursorPos(window_ptr, &x, &y);
        return {x, y};
    }

    double scroll_pos = 0;
    Application* application = nullptr;

private:
    static void keyCb(GLFWwindow* _window, int32_t _key, int32_t _scancode, int32_t _action, int32_t _mods);
    static void charCb(GLFWwindow* _window, uint32_t _scancode);
    static void scrollCb(GLFWwindow* _window, double _dx, double _dy);
    static void cursorPosCb(GLFWwindow* _window, double _mx, double _my);
    static void mouseButtonCb(GLFWwindow* _window, int32_t _button, int32_t _action, int32_t _mods);
    static void windowSizeCb(GLFWwindow* _window, int32_t _width, int32_t _height);
    static void dropFileCb(GLFWwindow* _window, int32_t _count, const char** _filePaths);

    GLFWwindow* window_ptr;

    double lastTime = 0;
};

static std::unique_ptr<Context> static_context;

void Context::keyCb(GLFWwindow* _window, int32_t _key, int32_t _scancode, int32_t _action, int32_t _mods)
{
    if (_key == GLFW_KEY_UNKNOWN)
    {
        return;
    }
    input::Modifier mods = translateKeyModifiers(_mods);
    input::Key::Enum key = translateKey(_key);
    bool down = (_action == GLFW_PRESS || _action == GLFW_REPEAT);
    if (static_context->application)
        static_context->application->onKey(key, mods, down);
}

void Context::charCb(GLFWwindow* _window, uint32_t _scancode)
{
    if (static_context->application)
        static_context->application->onChar(_scancode);
}

void Context::scrollCb(GLFWwindow* _window, double _dx, double _dy)
{
    double mx, my;
    glfwGetCursorPos(_window, &mx, &my);
    static_context->scroll_pos += _dy;
    if (static_context->application)
        static_context->application->onScroll(
            (int32_t) mx,
            (int32_t) my,
            (int32_t) static_context->scroll_pos
            );
}

void Context::cursorPosCb(GLFWwindow* _window, double _mx, double _my)
{
    if (static_context->application)
        static_context->application->cursorPos(
            (int32_t) _mx,
            (int32_t) _my,
            (int32_t) static_context->scroll_pos
            );
}

void Context::mouseButtonCb(GLFWwindow* _window, int32_t _button, int32_t _action, int32_t _mods)
{
    bool down = _action == GLFW_PRESS;
    double mx, my;
    glfwGetCursorPos(_window, &mx, &my);
    if (static_context->application)
        static_context->application->onMouseButton(
            (int32_t)mx,
            (int32_t)my,
            (int32_t)static_context->scroll_pos,
            translateMouseButton(_button),
            down);
}

void Context::windowSizeCb(GLFWwindow* _window, int32_t _width, int32_t _height)
{
    if (static_context->application)
        static_context->application->onWindowSizeChange(_width, _height);
}

void Context::dropFileCb(GLFWwindow* _window, int32_t _count, const char** _filePaths)
{
    if (static_context->application)
        for (int32_t ii = 0; ii < _count; ++ii)
        {
            static_context->application->onDropFile(_filePaths[ii]);
        }
}

namespace ev2::window {

void init(const Args& args) {
    static_context = std::make_unique<Context>();
    static_context->init();
}

double getFrameTime() {
    return static_context->getDelta();
}

bool frame() {
    return static_context->frame();
}

void setWindowTitle(const std::string& title) {
    static_context->setWindowTitle(title);
}

void setApplication(Application* app) {
    static_context->application = app;
}

glm::vec2 getCursorPosition() {
    return static_context->getCursor();
}

// void setMouseCursorVisible(bool visible) {
//     if(visible) {
//         glfwSetInputMode(window_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//     } else {
//         glfwSetInputMode(window_ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//     }
//     mouseCursorVisible = visible;
// }

// void updateMouseVel() {
//     if(!mouseCursorVisible) {
//         mouseVelocity = (mousePosition - prevMousePosition) * (double)deltaTime;
//     } else {
//         mouseVelocity = {};
//     }
//     prevMousePosition = mousePosition;
// }

}