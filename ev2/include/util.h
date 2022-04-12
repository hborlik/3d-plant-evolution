/**
 * @file util.h
 * @brief 
 * @date 2022-04-11
 * 
 */
#ifndef EV_UTIL_H
#define EV_UTIL_H

#include <string>
#include <type_traits>
#include <typeinfo>

namespace ev2::util {

std::string name_demangle(const std::string& mangled_name) noexcept;

/**
 * @brief Attempt to demangle a compiler generated name
 * 
 * @tparam T 
 * @return std::string 
 */
template<typename T>
inline std::string type_name() noexcept {
    return name_demangle(typeid(T).name());
}

}

#endif // EV_UTIL_H