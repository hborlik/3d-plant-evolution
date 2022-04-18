
#include <tests.h>

#include <ev.h>
#include <window.h>

int main(int argc, char *argv[]) {
    ev2::Args args{argc, argv};

    ev2::EV2_init(args);
    ev2::window::setWindowTitle("Testing");

    shader_tests();
}