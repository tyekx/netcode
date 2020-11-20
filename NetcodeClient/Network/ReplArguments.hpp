#pragma once

#include "ReplDesc.h"
#include "Replicator.hpp"

template<typename Component, typename TValueType>
struct ComponentValueAccessor {
	using ValueType = TValueType;
	
	ValueType Component:: * member;

	ComponentValueAccessor(ValueType Component::* memPtr) : member { memPtr } { }
	
	const ValueType & Read(GameObject* gameObject) const {
		Component * component = gameObject->GetComponent<Component>();
		return component->*member;
	}

	ValueType & Write(GameObject * gameObject) const {
		Component * component = gameObject->GetComponent<Component>();
		return component->*member;
	}
};

template<typename BaseType, typename TValueType>
struct StatefulValueAccessor {
	using ValueType = TValueType;
	
	BaseType * basePtr;
	ValueType BaseType:: * member;

	StatefulValueAccessor(BaseType * bPtr, ValueType BaseType:: * memPtr) : basePtr{ bPtr }, member{ memPtr } {

	}

	const ValueType & Read(GameObject * gameObject) const {
		UNREFERENCED_PARAMETER(gameObject);
		return basePtr->*member;
	}

	ValueType & Write(GameObject * gameObject) const {
		UNREFERENCED_PARAMETER(gameObject);
		return basePtr->*member;
	}
};

template<typename AccessorType>
struct AsIsReplArgument : public ReplArgumentBase {
	using PrimaryType = typename AccessorType::ValueType;
	
	AccessorType accessor;

	AsIsReplArgument(ReplType type, const AccessorType & accessor) : ReplArgumentBase{ type }, accessor { accessor } {

	}

	virtual uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> view) const override {
		return Replicator<PrimaryType>::QueryReplicatedSize(view);
	}

	virtual uint32_t GetReplicatedSize(GameObject* gameObject) const override {
		return Replicator<PrimaryType>::GetReplicatedSize(accessor.Read(gameObject));
	}

	virtual uint32_t Write(GameObject * gameObject, Netcode::MutableArrayView<uint8_t> dst) const override {
		return Replicator<PrimaryType>::Replicate(dst, accessor.Read(gameObject));
	}

	virtual uint32_t Read(GameObject * gameObject, Netcode::ArrayView<uint8_t> src) override {
		return Replicator<PrimaryType>::Replicate(accessor.Write(gameObject), src);
	}
};

template<typename Component, typename ValueType>
auto ReplicateAsIsFromComponent(ReplType type, ValueType Component::* memPtr) {
	return new AsIsReplArgument<ComponentValueAccessor<Component, ValueType>>{ type,
		ComponentValueAccessor<Component, ValueType>{ memPtr }
	};
}

template<typename BaseType, typename ValueType>
auto ReplicateAsIsFromState(ReplType type, BaseType * state, ValueType BaseType::* memPtr) {
	return new AsIsReplArgument<StatefulValueAccessor<BaseType, ValueType>>{ type,
		StatefulValueAccessor<BaseType, ValueType>{ state, memPtr }
	};
}

template<typename Accessor, typename ReplicatedType>
struct TransformedReplArgument : public ReplArgumentBase {
	using PrimaryType = typename Accessor::ValueType;
	using FTransform = ReplicatedType(*)(const PrimaryType &);
	using FInvTransform = PrimaryType(*)(const ReplicatedType &);

	Accessor accessor;
	
	FTransform transform;
	FInvTransform invTransform;

	TransformedReplArgument(ReplType type, const Accessor & accessor,
		FTransform transformFunction, FInvTransform invTransformFunction) : ReplArgumentBase{ type },
		accessor{ accessor }, transform{ transformFunction }, invTransform{ invTransformFunction } {

	}

	virtual uint32_t GetReplicatedSize(GameObject* gameObject) const override {
		ReplicatedType transformedValue = transform(accessor.Read(gameObject));
		return Replicator<ReplicatedType>::GetReplicatedSize(transformedValue);
	}

	virtual uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> view) const override {
		return Replicator<ReplicatedType>::QueryReplicatedSize(view);
	}

	virtual uint32_t Write(GameObject * gameObject, Netcode::MutableArrayView<uint8_t> dst) const override {
		ReplicatedType transformedValue = transform(accessor.Read(gameObject));
		return Replicator<ReplicatedType>::Replicate(dst.Data(), transformedValue);
	}

	virtual uint32_t Read(GameObject * gameObject, Netcode::ArrayView<uint8_t> src) override {
		ReplicatedType transformedValue;
		const uint32_t readSize = Replicator<ReplicatedType>::Replicate(transformedValue, src.Data());
		accessor.Write(gameObject) = invTransform(transformedValue);
		return readSize;
	}
};

template<typename Component, typename ValueType, typename ReplicatedType>
auto ReplicateTransformedFromComponent(ReplType type, ValueType Component::* member,
	ReplicatedType (*fTransform)(const ValueType &),
	ValueType (*fInvTransform)(const ReplicatedType &)) {

	return new TransformedReplArgument<ComponentValueAccessor<Component, ValueType>, ReplicatedType>{
		type,
		ComponentValueAccessor<Component, ValueType>{ member },
		fTransform,
		fInvTransform
	};
}

template<typename BaseType, typename ValueType, typename ReplicatedType>
auto ReplicateTransformedFromState(ReplType type, BaseType * state, ValueType BaseType:: * memPtr,
	ReplicatedType(*fTransform)(const ValueType &),
	ValueType(*fInvTransform)(const ReplicatedType &)) {
	
	return new TransformedReplArgument<StatefulValueAccessor<BaseType, ValueType>, ReplicatedType>{ type,
		StatefulValueAccessor<BaseType, ValueType>{ state, memPtr },
		fTransform,
		fInvTransform
	};
}
