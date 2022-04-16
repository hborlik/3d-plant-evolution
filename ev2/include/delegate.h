/*
 * delegate.h
 *
 * Revisions:
 *	Oct 3,	2018: Created
 *	Aug 21,	2019: Added lambda support
 *
 * Based on:
 * https://www.codeproject.com/Articles/1170503/The-Impossibly-Fast-Cplusplus-Delegates-Fixed
 *
 * todo:
 * test lambda implementation
 * assignment operation
 * move?
 * shared_ptr support
 */

#ifndef _DELEGATE_H_
#define _DELEGATE_H_

namespace ev2::util {

template<typename T>
class delegate;

/** This wraps a pointer to a templated static function that calls a function 
 *  or member function, based on input arguments. 
 */
template<typename Ret, typename ...Params>
class delegate<Ret(Params...)> {
public:
	using stub_type = Ret(*)(void* this_ptr, Params&&...);

	delegate() = default;

	/*!
	 * @brief copy constructor
	 */
	delegate(const delegate& other) {
		this->object = other.object;
		this->method_ptr = other.method_ptr;
	}

	/*!
	 * @brief Check if the function pointer is valid.
	 *
	 * @retval Is the delegate pointer null
	 */
	inline bool isNull() const noexcept {
		return method_ptr == nullptr;
	}

	inline bool operator==(const delegate& other) const noexcept {
		return other.method_ptr == method_ptr && other.object == object;
	}
	inline bool operator!=(const delegate& other) const noexcept {
		return !operator==(other);
	}


	/**!
	 * @brief Allow checking for validity by casting to a bool
	 * 
	 * @retval Is the delegate pointer not null (valid)
	 */
	inline explicit operator bool() const noexcept {
		return !isNull();
	}

	/**!
	 * @brief Assignment to nullptr
	 */
	inline delegate<Ret(Params...)>& operator=(decltype(nullptr) fn) noexcept {
		method_ptr = nullptr;
		object = nullptr;
		return *this;
	}

	/*!
	 * @brief Executes function pointer
	 *
	 * @param params variadic arguments
	 * @retval specified function return value
	 */
	Ret operator()(Params... params) const {
		return (*method_ptr)(object, static_cast<Params&&>(params)...);
	}

	/*!
	 * @brief Create a delegate from a member function with type Ret(T::*TMethod)(Params...)
	 *
	 * @retval delegate created
	 */
	template<typename T, Ret(T::*TMethod)(Params...)>
	static delegate create(T* object) {
		return delegate(object, method_stub<T, TMethod>);
	}

	/*!
	 * @brief Create a delegate from a const qualified member function with type Ret(T::*TMethod)(Params...) const
	 *
	 * @retval delegate created
	 */
	template<typename T, Ret(T::*TMethod)(Params...) const>
	static delegate create(const T* object) {
		return delegate(object, method_stub_const<T, TMethod>);
	}

	/*!
	 * @brief Create a delegate from a non-member function with type Ret (*TMethod)(Params...)
	 *
	 * @retval delegate created
	 */
	template<Ret (*TMethod)(Params...)>
	static delegate create() {
		return delegate(nullptr, function_stub<TMethod>);
	}

	/*!
	 * @brief Create a delegate from a lambda type
	 *
	 * @retval delegate created
	 */
	template<typename Lambda>
	static delegate create(const Lambda& object) {
		return delegate((void*)&object, lambda_stub<Lambda>);
	}

private:

	delegate(void* this_ptr, stub_type stub) :
			object(this_ptr), method_ptr(stub) {
	}

	/* In the templated static funtions below, the variadic rvalue references automatically
	 * expand to & or && depending on the types of arguments passed to the function */

	/* function call to member function */
	template<typename T, Ret (T::*TMethod)(Params...)>
	static Ret method_stub(void* this_ptr, Params&&... params) {
		T* obj = static_cast<T*>(this_ptr);
		return (obj->*TMethod)(static_cast<Params&&>(params)...);
	}

	/* function call to const member function */
	template<typename T, Ret (T::*TMethod)(Params...) const>
	static Ret method_stub_const(void* this_ptr, Params&&... params) {
		T* const obj = static_cast<T*>(this_ptr);
		return (obj->*TMethod)(static_cast<Params&&>(params)...);
	}

	/* function call to static function */
	template<Ret (*TMethod)(Params...)>
	static Ret function_stub(void* this_ptr, Params&&... params) {
		return (TMethod)(static_cast<Params&&>(params)...);
	}

	/* lambda function stub */
	template<typename Lambda>
	static Ret lambda_stub(void* this_ptr, Params&&... params) {
		Lambda* obj = static_cast<Lambda*>(this_ptr);
		// Lambda gaurenteed to have operator() defined
		return (obj->operator())(static_cast<Params&&>(params)...);
	}

	void* object = nullptr;
	stub_type method_ptr = nullptr;
};

}

#endif /* _DELEGATE_H_ */
