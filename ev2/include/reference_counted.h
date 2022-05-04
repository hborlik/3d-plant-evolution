/**
 * @file reference_counted.h
 * @brief 
 * @date 2022-04-21
 * 
 */
#ifndef EV2_REFERENCE_COUNTED_H
#define EV2_REFERENCE_COUNTED_H

#include <stdint.h>

namespace ev2 {

class ReferenceCountedBase {
public:
    virtual ~ReferenceCountedBase() = default;

    uint32_t count = 0;

    void decrement() {
        count--;
        // TODO offload deletion to a cleanup thread?
        if (count == 0)
            delete this;
    }

    void increment() noexcept {count++;}
};

template<typename T>
class ReferenceCounted;

template<typename T>
struct Ref {

    Ref() = default;
    Ref(T* obj);
    ~Ref();

    Ref(const Ref<T>&) noexcept;
    Ref(Ref<T>&&) noexcept;

    Ref<T>& operator=(const Ref<T>& o) noexcept;

    T& operator*() const noexcept {
        return *this->operator->();
    }

    T* operator->() const noexcept {
        return dynamic_cast<T*>(object);
    }

    T* get() noexcept {
        return this->operator->();
    }

    bool operator==(const Ref<T>& o) noexcept {return o.object == object;}

    void clear();

private:
    template<typename B>
    friend Ref<B> ref_cast(const Ref<T>& ref_a) {
        B *obj = dynamic_cast<B*>(ref_a.object);
        return Ref<B>{obj};
    }

    T* object = nullptr;
};

template<typename B, typename T>
Ref<B> ref_cast(const Ref<T>& ref_a);

template<typename T>
class ReferenceCounted : public ReferenceCountedBase {
public:
    ReferenceCounted() = default;
    virtual ~ReferenceCounted() = default;

    ReferenceCounted(const ReferenceCounted&) = delete;
    ReferenceCounted<T> operator=(const ReferenceCounted<T>& o) = delete;
    ReferenceCounted<T> operator=(ReferenceCounted<T>&& o) = delete;

    Ref<T> get_ref() noexcept;

    uint32_t get_ref_count() const noexcept {return count;}

};

template<typename T>
Ref<T> ReferenceCounted<T>::get_ref() noexcept {
    return {this};
}

template<typename T>
Ref<T>::Ref(const Ref<T>& o) noexcept {
    *this = o;
}

template<typename T>
Ref<T>::Ref(Ref<T>&& o) noexcept {
    object = o.object;
    o.object = nullptr;
}

template<typename T>
Ref<T>& Ref<T>::operator=(const Ref<T>& o) noexcept {
    if (object)
        object->decrement();
    object = o.object;
    if (object)
        object->increment();
    return *this;
}

template<typename T>
Ref<T>::Ref(T* obj) : object{obj} {
    if (object)
        object->increment();
}

template<typename T>
Ref<T>::~Ref() {
    if (object)
        object->decrement();
}

template<typename T>
void Ref<T>::clear() {
    if (object)
        object->decrement();
    object = nullptr;
}

template<typename T, typename... Args>
Ref<T> make_referenced(Args&&... args) {
    T *obj = new T(std::forward<Args&&>(args)...);
    return Ref<T>{obj};
}

}

#endif // PARAKEET_REFERENCE_COUNTED_H