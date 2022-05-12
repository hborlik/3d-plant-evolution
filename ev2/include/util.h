/**
 * @file util.h
 * @brief 
 * @date 2022-04-11
 * 
 */
#ifndef EV2_UTIL_H
#define EV2_UTIL_H

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

// variadic min and max functions

template<typename T>
inline T vmin(T&&t)
{
    return std::forward<T>(t);
}

template<typename T0, typename T1, typename... Ts>
inline typename std::common_type<T0, T1, Ts...>::type vmin(T0&& val1, T1&& val2, Ts&&... vs) {
    if (val2 < val1)
        return vmin(val2, std::forward<Ts>(vs)...);
    else
        return vmin(val1, std::forward<Ts>(vs)...);
}

template<typename T>
inline T vmax(T&&t)
{
    return std::forward<T>(t);
}

template<typename T0, typename T1, typename... Ts>
inline typename std::common_type<T0, T1, Ts...>::type vmax(T0&& val1, T1&& val2, Ts&&... vs) {
    if (val2 > val1)
        return vmax(val2, std::forward<Ts>(vs)...);
    else
        return vmax(val1, std::forward<Ts>(vs)...);
}

}

#endif // EV2_UTIL_H