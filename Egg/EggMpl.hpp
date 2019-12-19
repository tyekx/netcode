#pragma once

#include <iostream>
#include <tuple>

/*!!!!!!!!!!!!!!!
V2 Meta Programming Library
BEWARE. Here be dragons.
!!!!!!!!!!!!!!!*/


/*
Chapter 1. Introduction, MPL 101

If you still reading, then I assume you want to understand the things here.
You will need to understand to concept:
Storing types, Unpacking types.

1. To store types I used std::tuple<T...>

2. To unpack them, I always used a template specialization as following:

template<typename ... T>
struct ToBeSpecialized;

template<typename ... T>
struct ToBeSpecialized< std::tuple<T...> > { // <--------- MAGIC HAPPENED HERE, we can access T...

};

------------------------
Chapter 2. Interface of this MPL:

Before we dive into each functionality of this MPL, you must understand that anything here having a "Impl" suffix,
is harder to use, always use the one without the suffix, that would be the "public" interface.

- TupleContainsType<NEEDLE, TUPLE>::value : checks if needle is part of the tuple
- TupleIndexOf<NEEDLE, TUPLE>::value : gets the index of the NEEDLE, sizeof...(T) if not found
- TupleCreateMask<TUPLE, T...>::value : gets an UI32, every bit is 1 where tuple has T, using TupleIndexOf.
	example: let U = std::tuple<int, float, bool>, then TupleCreateMask<U, float, bool>::value == 0b110
- TupleCreateMask<TUPLE, TUPLE2>::value : same as before except type storage instead of variadic template parameters
- TupleSizeofSum<TUPLE>::value : gets the sizeof() including every type found in the tuple.
- TupleOffsetOf<NEEDLE, TUPLE>::value : gets the offset in bytes if the tuple elements were laid next to each other in memory
- TupleAppend<U, TUPLE>::type : returns a std::tuple<T..., U> where T... were already placed there, if U is void than the tuple doesnt change
- TuplePrepend<U, TUPLE>::type : returns a std::tuple<U, T...>, if U is void than the tuple doesnt change
- TupleApply<F, TUPLE>::type : returns a std::tuple<F<T1>, F<T2>, ...>, F must be a template<typename T>
- TupleRename<F, TUPLE>::type : takes the T... arguments from the tuple, and gives it to F. F<T...> will be the result
- TupleSelect<UI32, TUPLE>::type : selects the types from a tuple where the corresponding indexed bit is 1.
	example: let U = std::tuple<int, float, bool>, let I32 = 0b110, then TupleSelect<I32, U>::type  == std::tuple<float, bool>


--> Engine specific helpers and functions that is tightly connected to the API.

- TupleInvokeEvery<TUPLE> : for every tuple calls the Invoke method with a specifiable Object parameter
- HandleProxy<TUPLE> : handles the injection part of the function call, basically abusing the ellipsis (...) operator
					   to get the proper components from the Object.
- CompositeObjectDestructor<TUPLE> : invokes a destructor based on a runtime signature

Again, the 2 last is very specific and should be only used by the engine.


------------------------
Chapter 3. Understanding anything here:

What? Still reading? You are presistent, here is a :cookie: ! Anyhow, I cant help you read code, so thats on you, but
I can give you a general feel of the structure of _every_ piece here.

1. Declaration: It always starts with a bodyless struct
2. Exit condition(s): The last step of the recursion happens here
3. Recrusion (if any): Define a recursive step to make the C++ compiler work instead of us
					   If no recursion is needed, then you'll find the implementation in this section

Sorry, from now on you really need to start digging deeper and read the code, I tried to be as verbal as possible with
my templates, so I hope I was successful, have fun!

*/

// whenever we need refer to a signature, use this, signatures must be able to have bitwise operations like XOR, AND, OR and shift operations
using SignatureType = uint64_t;


template<typename NEEDLE, typename TUPLE>
struct TupleContainsType;

template<typename NEEDLE>
struct TupleContainsType<NEEDLE, std::tuple<> > {
	constexpr static bool value = false;
};

template<typename NEEDLE, typename HEAD, typename ... TAIL>
struct TupleContainsType<NEEDLE, std::tuple<HEAD, TAIL...> > {
	constexpr static bool value = TupleContainsType<NEEDLE, std::tuple<TAIL...> >::value;
};

template<typename NEEDLE, typename ... TAIL>
struct TupleContainsType<NEEDLE, std::tuple<NEEDLE, TAIL...> > {
	constexpr static bool value = true;
};





template<typename NEEDLE, typename TUPLE>
struct TupleIndexOf;

template<typename NEEDLE>
struct TupleIndexOf<NEEDLE, std::tuple<> > {
	constexpr static uint32_t value = 0;
};

template<typename NEEDLE, typename HEAD, typename ... TAIL>
struct TupleIndexOf<NEEDLE, std::tuple<HEAD, TAIL...> > {
	constexpr static uint32_t value = 1 + TupleIndexOf<NEEDLE, std::tuple<TAIL...> >::value;
};

template<typename NEEDLE, typename ... TAIL>
struct TupleIndexOf<NEEDLE, std::tuple<NEEDLE, TAIL...> > {
	constexpr static uint32_t value = 0;
};




template<typename TUPLE_1, typename TUPLE_2>
struct TupleMerge;

template<typename ... T, typename ... U>
struct TupleMerge<std::tuple<T...>, std::tuple<U...>> {
	using type = std::tuple<T..., U...>;
};



template<typename TUPLE, typename ... T>
struct TupleCreateMask;

template<typename TUPLE>
struct TupleCreateMask<TUPLE> {
	constexpr static SignatureType value = 0;
};

template<typename TUPLE>
struct TupleCreateMask<TUPLE, std::tuple<> > {
	constexpr static SignatureType value = 0;
};

template<typename TUPLE, typename HEAD, typename ... TAIL>
struct TupleCreateMask<TUPLE, HEAD, TAIL...> {
	constexpr static SignatureType value = (1LLU << TupleIndexOf<HEAD, TUPLE>::value) | TupleCreateMask<TUPLE, TAIL...>::value;
};

template<typename TUPLE, typename HEAD, typename ... TAIL>
struct TupleCreateMask<TUPLE, std::tuple<HEAD, TAIL...> > {
	constexpr static SignatureType value = (1LLU << TupleIndexOf<HEAD, TUPLE>::value) | TupleCreateMask<TUPLE, std::tuple<TAIL...> >::value;
};


template<typename NEEDLE, typename ... T>
struct TupleOffsetOf;

template<typename NEEDLE>
struct TupleOffsetOf<NEEDLE, std::tuple<> > {
	constexpr static uint32_t value = 0;
};

template<typename NEEDLE, typename HEAD, typename ... TAIL>
struct TupleOffsetOf<NEEDLE, std::tuple<HEAD, TAIL...> > {
	constexpr static uint32_t value = sizeof(HEAD) + TupleOffsetOf<NEEDLE, std::tuple<TAIL...> >::value;
};

template<typename NEEDLE, typename ... TAIL>
struct TupleOffsetOf<NEEDLE, std::tuple<NEEDLE, TAIL...> > {
	constexpr static uint32_t value = 0;
};





template<typename ... T>
struct TupleSizeofSum;

template<>
struct TupleSizeofSum< std::tuple<> > {
	constexpr static uint32_t value = 0;
};

template<typename HEAD, typename ... TAIL>
struct TupleSizeofSum< std::tuple<HEAD, TAIL...> > {
	constexpr static uint32_t value = sizeof(HEAD) + TupleSizeofSum< std::tuple<TAIL...> >::value;
};





template<typename ... T>
struct TupleCountOf;

template<>
struct TupleCountOf< std::tuple<> > {
	constexpr static uint32_t value = 0;
};

template<typename HEAD, typename ... TAIL>
struct TupleCountOf< std::tuple<HEAD, TAIL...> > {
	constexpr static uint32_t value = 1U + sizeof...(TAIL);
};





template<SignatureType i, typename ... T>
struct TupleMaxMaskValueImpl;

template<SignatureType i>
struct TupleMaxMaskValueImpl< i, std::tuple<> > {
	constexpr static SignatureType value = 0;
};

template<SignatureType i, typename HEAD, typename ... TAIL>
struct TupleMaxMaskValueImpl<i, std::tuple<HEAD, TAIL...> > {
	constexpr static SignatureType value = (1ULL << i) | TupleMaxMaskValueImpl<(i + 1), std::tuple< TAIL...> >::value;
};

template<typename TUPLE_TYPE>
struct TupleMaxMaskValue {
	constexpr static SignatureType value = TupleMaxMaskValueImpl<0ULL, TUPLE_TYPE>::value;
};



template<typename... TUPLE>
struct TupleAppend;

template<typename ITEM, typename ... TUPLE>
struct TupleAppend<ITEM, std::tuple<TUPLE...> > {
	using type = std::tuple<TUPLE..., ITEM>;
};

template<typename ... TUPLE>
struct TupleAppend<void, std::tuple<TUPLE...> > {
	using type = std::tuple<TUPLE...>;
};





template<typename... TUPLE>
struct TuplePrepend;

template<typename ITEM, typename ... TUPLE>
struct TuplePrepend<ITEM, std::tuple<TUPLE...> > {
	using type = std::tuple<ITEM, TUPLE...>;
};

template<typename ... TUPLE>
struct TuplePrepend<void, std::tuple<TUPLE...> > {
	using type = std::tuple<TUPLE...>;
};





template<template <typename> typename TEMPLATE_TO_APPLY, typename ... TUPLE_TYPES>
struct TupleApply;

template<template <typename> typename TEMPLATE_TO_APPLY>
struct TupleApply<TEMPLATE_TO_APPLY, std::tuple<> > {
	using type = std::tuple<>;
};

template<template <typename> typename TEMPLATE_TO_APPLY, typename HEAD, typename ... TAIL>
struct TupleApply<TEMPLATE_TO_APPLY, std::tuple<HEAD, TAIL...> > {
	using type = typename TuplePrepend<TEMPLATE_TO_APPLY<HEAD>, typename TupleApply<TEMPLATE_TO_APPLY, std::tuple<TAIL...> >::type >::type;
};





template<template<typename...> typename CANDIDATE, typename ... T>
struct TupleRename;

template<template<typename...> typename CANDIDATE, typename ... T>
struct TupleRename< CANDIDATE, std::tuple<T...> > {
#ifdef _MSC_VER
	using type = typename CANDIDATE<T...>;
#else
	using type = CANDIDATE<T...>;
#endif
};





template<SignatureType C, SignatureType R, typename ... T>
struct TupleSelectImpl;

template<SignatureType C, SignatureType R>
struct TupleSelectImpl<C, R, std::tuple<> > {
	using type = std::tuple<>;
};

template<SignatureType R, typename HEAD, typename ... TAIL>
struct TupleSelectImpl<0, R, std::tuple<HEAD, TAIL...> > {
	constexpr static SignatureType NEXT_R = (R >> 1);
	constexpr static SignatureType NEXT_C = NEXT_R & 0x1;
	using type = typename TuplePrepend< void, typename TupleSelectImpl<NEXT_C, NEXT_R, std::tuple<TAIL...> >::type >::type;
};

template<SignatureType R, typename HEAD, typename ... TAIL>
struct TupleSelectImpl<1, R, std::tuple<HEAD, TAIL...> > {
	constexpr static SignatureType NEXT_R = (R >> 1);
	constexpr static SignatureType NEXT_C = NEXT_R & 0x1;
	using type = typename TuplePrepend< HEAD, typename TupleSelectImpl<NEXT_C, NEXT_R, std::tuple<TAIL...> >::type >::type;
};

template<SignatureType MASK, typename TUPLE_TYPE>
struct TupleSelect {
	using type = typename TupleSelectImpl<MASK & 0x1, MASK, TUPLE_TYPE>::type;
};


template<typename OBJECT_TYPE, typename TUPLE_START, typename ... T>
struct TupleInvokeEveryImpl;

template<typename OBJECT_TYPE, typename TUPLE_START>
struct TupleInvokeEveryImpl<OBJECT_TYPE, TUPLE_START, std::tuple<> > {
	inline static void Invoke(OBJECT_TYPE & object, TUPLE_START & tuple) {
		//empty, exit condition
	}
};

template<typename OBJECT_TYPE, typename TUPLE_START, typename HEAD, typename ... TAIL>
struct TupleInvokeEveryImpl<OBJECT_TYPE, TUPLE_START, std::tuple<HEAD, TAIL...> > {
	inline static void Invoke(OBJECT_TYPE & object, TUPLE_START & tuple) {
		(std::get<HEAD>(tuple).HEAD::Invoke)(object);
		TupleInvokeEveryImpl<OBJECT_TYPE, TUPLE_START, std::tuple<TAIL...> >::Invoke(object, tuple);
	}
};

template<typename OBJECT_TYPE, typename TUPLE>
struct TupleInvokeEvery {
	inline static void Invoke(OBJECT_TYPE & object, TUPLE & tuple) {
		TupleInvokeEveryImpl<OBJECT_TYPE, TUPLE, TUPLE>::Invoke(object, tuple);
	}
};



template<typename ... T>
struct CompositeObjectDestructor;

template<>
struct CompositeObjectDestructor< std::tuple<> > {
	static void Invoke(SignatureType signature, uint8_t * memory) {
		// exit condition
	}
};

template<typename HEAD, typename ... TAIL>
struct CompositeObjectDestructor< std::tuple<HEAD, TAIL...> > {
	static void Invoke(SignatureType signature, uint8_t * memory) {
		if((signature & 1) == 1) {
			((HEAD*)memory)->~HEAD();
		}
		CompositeObjectDestructor<std::tuple<TAIL...>>::Invoke(signature >> 1, memory + sizeof(HEAD));
	}
};


template<typename ... T>
struct InjectComponents;

template<typename SYSTEM_T, typename OBJECT_T, typename ... COMPONENT_TYPES, typename ... ADDITIONAL_ARGS>
struct InjectComponents< SYSTEM_T, OBJECT_T, std::tuple<COMPONENT_TYPES...>, ADDITIONAL_ARGS... > {

	using FUNCTION_T = void (SYSTEM_T::*)(OBJECT_T *, COMPONENT_TYPES *..., ADDITIONAL_ARGS...);

	static void Invoke(SYSTEM_T & ref, FUNCTION_T func, OBJECT_T * obj, ADDITIONAL_ARGS ... args) {
		((ref).*(func))(obj, obj->template GetComponent<COMPONENT_TYPES>()..., args...);
	}
};
