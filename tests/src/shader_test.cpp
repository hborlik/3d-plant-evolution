#include <iostream>
#include <filesystem>
#include <renderer/shader.h>
#include <ev.h>

using namespace ev2;
namespace fs = std::filesystem;

int main() {
    ev2::EV2_init(ev2::Args{}, fs::path{"asset"});

    ev2::ShaderPreprocessor prep{fs::path{"asset"} / "shader"};
    prep.load_includes();


    Program p{"test"};
    p.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "geometry_instanced.glsl.vert", prep);
    p.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "geometry.glsl.frag", prep);
    p.link();

    std::cout << p << std::endl;

    ev2::EV2_shutdown();
}