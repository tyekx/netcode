#include "Systems.h"


template<typename ... T>
struct InjectSystemArgs;

template<typename SYSTEM_T, typename ... COMPONENT_TYPES, typename ... ADDITIONAL_ARGS>
struct InjectSystemArgs<SYSTEM_T, std::tuple<COMPONENT_TYPES...>, ADDITIONAL_ARGS... > {
	static void Invoke(SYSTEM_T & ref, GameObject * obj, ADDITIONAL_ARGS && ... args) {
		ref(obj, obj->template GetComponent<COMPONENT_TYPES>()..., std::forward<ADDITIONAL_ARGS>(args)...);
	}
};

template<typename ... T>
struct InjectComponents;

template<typename FUNC_T, typename ... COMPONENT_TYPES>
struct InjectComponents<FUNC_T, std::tuple<COMPONENT_TYPES...>> {
	static void Invoke(GameObject * gameObject, FUNC_T func) {
		func(gameObject->GetComponent<COMPONENT_TYPES>()...);
	}
};

template<typename FUNC_OWNER_T, typename FUNC_T, typename ... COMPONENT_TYPES>
struct InjectComponents<FUNC_OWNER_T, FUNC_T, std::tuple<COMPONENT_TYPES...>> {
	static void Invoke(GameObject * gameObject, FUNC_OWNER_T* ownerRef, FUNC_T func) {
		((*ownerRef).*func)(gameObject->GetComponent<COMPONENT_TYPES>()...);
	}
};




// cuts off the extra arguments, very specialized task
template<bool KeepGoing, typename ACC, typename ... T>
struct TupleKeepComponentsImpl;

template<bool Any, typename ACC>
struct TupleKeepComponentsImpl<Any, ACC, std::tuple<>> {
	using type = typename ACC;
};

template<typename ACC, typename HEAD, typename ... TAIL>
struct TupleKeepComponentsImpl <false, ACC, std::tuple<HEAD, TAIL...> > {
	using type = typename ACC;
};

template<typename ACC, typename HEAD, typename ... TAIL>
struct TupleKeepComponentsImpl <true, ACC, std::tuple<HEAD, TAIL...> > {
	constexpr static bool Contains = TupleContainsType< std::remove_pointer_t<HEAD>, AllComponents_T>::value;

	using type = typename TupleKeepComponentsImpl<Contains, typename std::conditional<Contains, typename TupleAppend<HEAD, ACC>::type, ACC>::type, std::tuple<TAIL...> >::type;
};

template<typename ... T>
struct TupleKeepComponents;

template<typename HEAD, typename ... TAIL>
struct TupleKeepComponents<std::tuple<HEAD, TAIL...>> {
	using type = typename TupleKeepComponentsImpl<true, std::tuple<>, std::tuple<HEAD, TAIL...>>::type;
};

template<typename T, typename SYSTEM, typename ... ADDITIONAL_ARGS>
void TryInvoke(T * obj, SYSTEM * system, ADDITIONAL_ARGS && ... args) {
	using FunctionType = decltype(&SYSTEM::operator());
	using ArgsTuple = typename FunctionReflection<FunctionType>::ArgsTuple;
	using MemberOf = typename FunctionReflection<FunctionType>::MemberOf;
	using Args = typename TupleSkipN<1, ArgsTuple>::type;
	using ComponentArgs = typename TupleKeepComponents<Args>::type;
	using ComponentTypes = typename TupleForEach<std::remove_pointer_t, ComponentArgs>::type;

	constexpr static SignatureType requiredComponents = TupleCreateMask<AllComponents_T, ComponentTypes>::value;

	if((requiredComponents & obj->GetSignature()) == requiredComponents) {
		InjectSystemArgs<MemberOf, T, ComponentTypes, ADDITIONAL_ARGS...>::Invoke(*system, obj, std::forward<ADDITIONAL_ARGS>(args)...);
	}
}


template<typename FUNC_T, typename FUNC_OWNER_T>
void TryInject(GameObject * gameObject, FUNC_T f, FUNC_OWNER_T * ownerRef) {
	using ComponentArgs = typename FunctionReflection<FUNC_T>::ArgsTuple;
	using ComponentTypes = typename TupleForEach<std::remove_pointer_t, ComponentArgs>::type;
	using MemberOf = typename FunctionReflection<FUNC_T>::MemberOf;

	if constexpr(std::is_same_v<MemberOf, void>) {
		InjectComponents<FUNC_T, ComponentTypes>::Invoke(gameObject, f);
	} else {
		static_assert(std::is_base_of_v<FUNC_OWNER_T, MemberOf> || std::is_same_v<FUNC_OWNER_T, MemberOf>,
			"Supplied function owner is invalid");

		InjectComponents<FUNC_OWNER_T, FUNC_T, ComponentTypes>::Invoke(gameObject, ownerRef, f);
	}

}

void TransformSystem::Run(GameObject * gameObject) {
	TryInvoke(gameObject, this);
}

void ScriptSystem::Run(GameObject * gameObject, float dt) {
	TryInvoke(gameObject, this, dt);
}

void RenderSystem::Run(GameObject * gameObject) {
	 TryInvoke(gameObject, this);
}

void AnimationSystem::Run(GameObject * gameObject, float dt) {
	TryInvoke(gameObject, this, dt);
}

void PhysXSystem::Run(GameObject * gameObject) {
	TryInvoke(gameObject, this);
}
