#pragma once

#include "GameObject.h"
#include <Egg/Modules.h>

using AllComponents_T = TupleMerge<Components_T, ExtensionComponents_T>::type;

class TransformSystem {
	static inline DirectX::XMMATRIX GetModelMatrix(Transform* transform) {
		DirectX::XMVECTOR posVector = DirectX::XMLoadFloat3(&transform->position);
		DirectX::XMVECTOR quatVector = DirectX::XMLoadFloat4(&transform->rotation);
		DirectX::XMVECTOR scaleVector = DirectX::XMLoadFloat3(&transform->scale);
		DirectX::XMFLOAT4 unitWValue{ 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMVECTOR unitW = DirectX::XMLoadFloat4(&unitWValue);

		return DirectX::XMMatrixAffineTransformation(scaleVector, unitW, quatVector, posVector);
	}
public:
	using RequiredComponents_T = std::tuple<Model, Transform>;

	static constexpr SignatureType Required() {
		return TupleCreateMask<Components_T, RequiredComponents_T>::value;
	}

	bool SignatureMatch(GameObject * gameObject) {
		return (Required() & gameObject->GetSignature()) == Required();
	}

	void Run(GameObject * gameObject) {
		if(SignatureMatch(gameObject)) {
			InjectComponents<TransformSystem, GameObject, RequiredComponents_T>::Invoke(*this, &TransformSystem::RunImpl, gameObject);
		}
	}

	void RunImpl(GameObject * gameObject, Model * model, Transform * transform) {
		DirectX::XMMATRIX modelMat = GetModelMatrix(transform);

		if(gameObject->Parent() != nullptr) {
			DirectX::XMMATRIX parentTransform = GetModelMatrix(gameObject->Parent()->GetComponent<Transform>());
			modelMat = DirectX::XMMatrixMultiply(parentTransform, modelMat);
		}


		DirectX::XMVECTOR determinant = DirectX::XMMatrixDeterminant(modelMat);
		DirectX::XMMATRIX invModelMat = DirectX::XMMatrixInverse(&determinant, modelMat);

		DirectX::XMStoreFloat4x4A(&model->perObjectData.Model, DirectX::XMMatrixTranspose(modelMat));
		DirectX::XMStoreFloat4x4A(&model->perObjectData.InvModel, DirectX::XMMatrixTranspose(invModelMat));
	}
};

class ScriptSystem {
public:
	using RequiredComponents_T = std::tuple<Script>;

	static constexpr SignatureType Required() {

		return TupleCreateMask<Components_T, RequiredComponents_T>::value;
	}

	bool SignatureMatch(GameObject * gameObject) {
		return (Required() & gameObject->GetSignature()) == Required();
	}

	void Run(GameObject * gameObject, float dt) {
		if(SignatureMatch(gameObject)) {
			InjectComponents<ScriptSystem, GameObject, RequiredComponents_T, float>::Invoke(*this, &ScriptSystem::RunImpl, gameObject, dt);
		}
	}

	void RunImpl(GameObject * gameObject, Script * script, float dt) {
		script->Update(dt);
	}
};

class RenderSystem {
public:
	using RequiredComponents_T = std::tuple<Model>;
	Egg::Module::IGraphicsModule * graphics;
	static constexpr SignatureType Required() {
		return TupleCreateMask<Components_T, RequiredComponents_T>::value;
	}

	bool SignatureMatch(GameObject * gameObject) {
		return (Required() & gameObject->GetSignature()) == Required();
	}

	void SetGraphics(Egg::Module::IGraphicsModule * graphicsModule) {
		graphics = graphicsModule;
	}

	void Run(GameObject * gameObject) {
		if(SignatureMatch(gameObject)) {
			InjectComponents<RenderSystem, GameObject, RequiredComponents_T >::Invoke(*this, &RenderSystem::RunImpl, gameObject);
		}
	}

	void RunImpl(GameObject * gameObject, Model * model) {
		for(const auto & i : model->meshes) {
		//	graphics->Record(i.mesh);
		}
	}
};



class AnimationSystem {
	Egg::MovementController * movementController;
public:
	using RequiredComponents_T = std::tuple<Model, Animation>;

	static constexpr SignatureType Required() {
		return TupleCreateMask<AllComponents_T, RequiredComponents_T>::value;
	}

	bool SignatureMatch(GameObject * gameObject) {
		return (Required() & gameObject->GetSignature()) == Required();
	}

	void Run(GameObject * gameObject, float dt) {
		if(SignatureMatch(gameObject)) {
			InjectComponents<AnimationSystem, GameObject, RequiredComponents_T, float>::Invoke(*this, &AnimationSystem::RunImpl, gameObject, dt);
		}
	}

	void SetMovementController(Egg::MovementController * movCtrl) {
		movementController = movCtrl;
	}

	void RunImpl(GameObject * gameObject, Model * model, Animation* anim, float dt) {
		if(model->boneData != nullptr && movementController != nullptr) {
			movementController->Update();
			anim->blackboard.Update(dt, movementController);
			anim->blackboard.CopyBoneDataInto(model->boneData->BindTransform);
		}
	}
};

