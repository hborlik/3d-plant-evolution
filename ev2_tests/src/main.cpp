
#include <tests.h>

#include <ev.h>

int main(int argc, char *argv[]) {
    ev2::Args args{argc, argv};

    ev2::EV_init(args);
    shader_tests();
}