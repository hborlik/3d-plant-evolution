#include "../../ev2/include/reference_counted.h"

#include <iostream>

struct Foo : public ev2::ReferenceCounted<Foo> {

};

struct Glue : public Foo {};

int main() {
    ev2::Ref<Glue> glue = ev2::make_referenced<Glue>();
    ev2::Ref<Foo> f = glue;
    ev2::Ref<Foo> g(f);

    std::cout << f._ref->count << std::endl;

    return 0;
}