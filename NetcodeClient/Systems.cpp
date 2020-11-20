#include "Systems.h"


template<typename ... T>
struct InjectSystemArgs;

template<typename SYSTEM_T, typename OBJ_TYPE, typename ... COMPONENT_TYPES, typename ... ADDITIONAL_ARGS>
struct InjectSystemArgs<SYSTEM_T, OBJ_TYPE, std::tuple<COMPONENT_TYPES...>, ADDITIONAL_ARGS... > {
	static void Invoke(SYSTEM_T & ref, OBJ_TYPE * obj, ADDITIONAL_ARGS && ... args) {
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
template<bool KeepGoing, typename ALL_COMPONENTS_T, typename ACC, typename ... T>
struct TupleKeepComponentsImpl;

template<bool Any, typename ALL_COMPONENTS_T, typename ACC>
struct TupleKeepComponentsImpl<Any, ALL_COMPONENTS_T, ACC, std::tuple<>> {
	using type = ACC;
};

template<typename ALL_COMPONENTS_T, typename ACC, typename HEAD, typename ... TAIL>
struct TupleKeepComponentsImpl <false, ALL_COMPONENTS_T, ACC, std::tuple<HEAD, TAIL...> > {
	using type = ACC;
};

template<typename ALL_COMPONENTS_T, typename ACC, typename HEAD, typename ... TAIL>
struct TupleKeepComponentsImpl <true, ALL_COMPONENTS_T, ACC, std::tuple<HEAD, TAIL...> > {
	constexpr static bool Contains = TupleContainsType< std::remove_pointer_t<HEAD>, ALL_COMPONENTS_T>::value;

	using type = typename TupleKeepComponentsImpl<Contains, ALL_COMPONENTS_T, typename std::conditional<Contains, typename TupleAppend<HEAD, ACC>::type, ACC>::type, std::tuple<TAIL...> >::type;
};

template<typename ... T>
struct TupleKeepComponents;

template<typename ALL_COMPONENTS_T, typename HEAD, typename ... TAIL>
struct TupleKeepComponents<ALL_COMPONENTS_T, std::tuple<HEAD, TAIL...>> {
	using type = typename TupleKeepComponentsImpl<true, ALL_COMPONENTS_T, std::tuple<>, std::tuple<HEAD, TAIL...>>::type;
};

template<typename T, typename SYSTEM, typename ... ADDITIONAL_ARGS>
void TryInvoke(T * obj, SYSTEM * system, ADDITIONAL_ARGS && ... args) {
	using FunctionType = decltype(&SYSTEM::operator());
	using ArgsTuple = typename FunctionReflection<FunctionType>::ArgsTuple;
	using MemberOf = typename FunctionReflection<FunctionType>::MemberOf;
	using Args = typename TupleSkipN<1, ArgsTuple>::type;
	using ComponentArgs = typename TupleKeepComponents<typename T::ComponentTypes, Args>::type;
	using ComponentTypes = typename TupleForEach<std::remove_pointer_t, ComponentArgs>::type;

	constexpr static SignatureType requiredComponents = TupleCreateMask<typename T::ComponentTypes, ComponentTypes>::value;

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

void ScriptSystem::Run(GameObject * gameObject, Netcode::GameClock * clock) {
	TryInvoke(gameObject, this, clock);
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

void LightSystem::Run(GameObject * gameObject) {
	TryInvoke(gameObject, this);
}
