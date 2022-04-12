#include <util.h>

#include <cxxabi.h>

namespace ev2::util {

std::string name_demangle(const std::string& mangled_name) noexcept {
    int status;
    char* demangled_name = abi::__cxa_demangle(mangled_name.c_str(), 0, 0, &status);
    std::string name{demangled_name};
    if (demangled_name)
        std::free(demangled_name);
    return name;
}

}