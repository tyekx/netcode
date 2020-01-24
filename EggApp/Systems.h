#pragma once

#include "GameObject.h"
#include <Egg/Modules.h>
#include "RenderPassTest.h"

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

	void Run(GameObject * gameObject);

	void operator()(GameObject * gameObject, Model * model, Transform * transform) {
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
	void Run(GameObject * gameObject, float dt);

	void operator()(GameObject * gameObject, Script * script, float dt) {
		script->Update(dt);
	}
};

class RenderSystem {
public:
	AppDefinedRenderer renderer;

	void CreatePermanentResources(Egg::Module::IGraphicsModule * graphics) {
		renderer.CreatePermanentResources(graphics);
	}

	void Run(GameObject * gameObject);

	void operator()(GameObject * gameObject, Model * model) {
		for(const auto & i : model->meshes) {
			if(model->boneData.get() != nullptr) {
				renderer.skinningPass_Input.Produced(RenderItem(i, &model->perObjectData, model->boneData.get()));
			} else {
				renderer.gbufferPass_Input.Produced(RenderItem(i, &model->perObjectData, model->boneData.get()));
			}
		}
	}
};



class AnimationSystem {
	Egg::MovementController * movementController;
public:
	void Run(GameObject * gameObject, float dt);

	void SetMovementController(Egg::MovementController * movCtrl) {
		movementController = movCtrl;
	}

	void operator()(GameObject * gameObject, Model * model, Animation* anim, float dt) {
		if(model->boneData != nullptr && movementController != nullptr) {
			movementController->Update();
			anim->blackboard.Update(dt, movementController);
			anim->blackboard.CopyBoneDataInto(model->boneData->BindTransform);
		}
	}
};

