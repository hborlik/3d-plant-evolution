/**
 * @file reference_counted.h
 * @brief 
 * @date 2022-04-21
 * 
 */
#ifndef EV2_REFERENCE_COUNTED_H
#define EV2_REFERENCE_COUNTED_H

#include <stdint.h>

#include <type_traits>
#include <utility>
#include <cstddef>

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

    template<typename _Yp, typename _Res = void>
	using _Assignable = typename
        std::enable_if<std::is_convertible<_Yp*, T*>::value, _Res>::type;

    Ref() noexcept = default;

    Ref(nullptr_t) noexcept : Ref() {}

    explicit Ref(ReferenceCounted<T>* obj) : _ref{dynamic_cast<T*>(obj)} {
        if (_ref)
            _ref->increment();
    }

    explicit Ref(T* obj) : _ref{obj} {
        if (_ref)
            _ref->increment();
    }

    template<typename _Yp, typename = _Assignable<_Yp>>
    explicit Ref(_Yp* obj) : _ref{obj} {
        if (_ref)
            _ref->increment();
    }

    Ref(const Ref<T>& o) noexcept {
        *this = o;
    }

    template<typename _Yp, typename = _Assignable<_Yp>>
    Ref(const Ref<_Yp>& o) noexcept {
        *this = o;
    }

    Ref(Ref<T>&& o) noexcept {
        _ref = o._ref;
        o._ref = nullptr;
    }

    template<typename _Yp, typename = _Assignable<_Yp>>
    Ref(Ref<_Yp>&& o) noexcept {
        _ref = o._ref;
        o._ref = nullptr;
    }

    ~Ref() {
        if (_ref)
            _ref->decrement();
    }

    T& operator*() const noexcept {
        return *this->operator->();
    }

    T* operator->() const noexcept {
        return _ref;
    }

    T* get() noexcept {
        return this->operator->();
    }

    bool operator==(const Ref<T>& o) noexcept {return o._ref == _ref;}

    void clear();

    Ref<T>& operator=(const Ref<T>& o) noexcept
	{
        if (_ref)
            _ref->decrement();
        _ref = o._ref;
        if (_ref)
            _ref->increment();
        return *this;
	}

    template<typename _Yp>
	_Assignable<_Yp, Ref<T>&>
	operator=(const Ref<_Yp>& o) noexcept
	{
        if (_ref)
            _ref->decrement();
        _ref = o._ref;
        if (_ref)
            _ref->increment();
        return *this;
	}

    Ref<T>& operator=(Ref<T>&& __r) noexcept
	{
        Ref(std::move(__r)).swap(*this);
        return *this;
	}

    template<typename _Yp>
	_Assignable<_Yp, Ref<T>&>
	operator=(Ref<_Yp>&& __r) noexcept
	{
        Ref(std::move(__r)).swap(*this);
        return *this;
	}

    operator bool() const noexcept {
        return _ref != nullptr;
    }

    template<class B> Ref<B> ref_cast() {
        B *obj = dynamic_cast<B*>(_ref);
        return Ref<B>{obj};
    }

    void swap(Ref<T>& r) noexcept {
        std::swap(r._ref, _ref);
    }

    T* _ref = nullptr;
};

template<typename T>
class ReferenceCounted : public ReferenceCountedBase {
public:
    ReferenceCounted() = default;
    virtual ~ReferenceCounted() = default;

    ReferenceCounted(const ReferenceCounted&) = delete;
    ReferenceCounted<T> operator=(const ReferenceCounted<T>& o) = delete;
    ReferenceCounted<T> operator=(ReferenceCounted<T>&& o) = delete;

    Ref<T> get_ref() noexcept {return Ref<T>{this};}

    uint32_t get_ref_count() const noexcept {return count;}

};

template<typename T>
void Ref<T>::clear() {
    if (_ref)
        _ref->decrement();
    _ref = nullptr;
}

template<typename T, typename... Args>
Ref<T> make_referenced(Args&&... args) {
    T *obj = new T(std::forward<Args&&>(args)...);
    return Ref<T>{obj};
}

}

#endif // PARAKEET_REFERENCE_COUNTED_H