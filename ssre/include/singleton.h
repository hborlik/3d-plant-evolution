/**
 * @file singleton.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-10-02
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once

#include <memory>
#include <ssre.h>

namespace ssre::util {

    template<typename T>
    class Singleton {
    public:
        template<typename ...Args>
        static void ConstructStatic(Args... arg) {
            SSRE_CHECK_THROW(!instance, "Static instance has already been constructed");
            instance = new T(std::move(arg)...);
        }

        static T& StaticInst() {
            SSRE_CHECK_THROW(instance, "Cannot get static instance of Singleton that has not been constructed");
            return *instance;
        }

        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        
    protected:
        Singleton() = default;
        // Note no virtual destructor, there should be no upcasting to this class, no polymorphic handling
        ~Singleton() = default;
    private:
        static T* instance;
    };

    template<typename T>
    T* Singleton<T>::instance = nullptr;

}