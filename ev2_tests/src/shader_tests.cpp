#include <shader.h>


#include <iostream>

int shader_tests() {
    ev2::Program prog{"test_prog"};
    prog.loadShader(ev2::gl::GLSLShaderType::FRAGMENT_SHADER, "asset/shader/basic.glsl.frag");
    prog.loadShader(ev2::gl::GLSLShaderType::VERTEX_SHADER, "asset/shader/basic.glsl.vert");

    prog.link();
    
    std::cout << prog;

    return 0;
}