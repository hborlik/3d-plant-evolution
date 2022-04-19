#include <shader.h>


#include <iostream>

namespace fs = std::filesystem;

int shader_tests() {
    ev2::Program prog{"test_prog"};
    fs::path shader_dir = fs::path("asset")/"shader"; 
    prog.loadShader(ev2::gl::GLSLShaderType::FRAGMENT_SHADER, shader_dir/"basic.glsl.frag");
    prog.loadShader(ev2::gl::GLSLShaderType::VERTEX_SHADER, shader_dir/"basic.glsl.vert");

    prog.link();
    
    std::cout << prog;

    return 0;
}