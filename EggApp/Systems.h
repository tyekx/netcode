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
			anim->blackboard.CopyToRootDataInto(model->boneData->ToRootTransform);
		}
	}
};

class PhysXSystem {
	void UpdateShapeLocalPose(physx::PxShape* shape, Model* model, const ColliderShape & c) {
		DirectX::XMMATRIX toRoot = DirectX::XMLoadFloat4x4A(&model->boneData->ToRootTransform[c.boneReference]);
		DirectX::XMVECTOR rotQ = DirectX::XMLoadFloat4(&c.localRotation);

		DirectX::CXMMATRIX T = DirectX::XMMatrixTranslation(c.localPosition.x, c.localPosition.y, c.localPosition.z);
		DirectX::CXMMATRIX R = DirectX::XMMatrixRotationQuaternion(rotQ);

		toRoot = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(R, T), DirectX::XMMatrixTranspose(toRoot));

		DirectX::XMFLOAT4X4 res;
		DirectX::XMStoreFloat4x4(&res, toRoot);

		DirectX::XMFLOAT3 tr{
			res._41,
			res._42,
			res._43
		};

		toRoot = DirectX::XMLoadFloat4x4(&res);

		rotQ = DirectX::XMQuaternionRotationMatrix(toRoot);
		DirectX::XMFLOAT4 rot;
		DirectX::XMStoreFloat4(&rot, rotQ);

		physx::PxTransform pxT{ ToPxVec3(tr), ToPxQuat(rot) };
		shape->setLocalPose(pxT);
	}

	void UpdateBoneAttachedShapes(Model * model, Collider * collider) {

		if(auto * rigidBody = collider->actorRef->is<physx::PxRigidDynamic>()) {
			constexpr static uint32_t SHAPES_BUFFER_SIZE = 8;

			physx::PxShape * shapes[SHAPES_BUFFER_SIZE] = {};
			uint32_t shapesCount = rigidBody->getNbShapes();

			uint32_t idx = 0;
			while(idx < shapesCount) {
				uint32_t written = rigidBody->getShapes(shapes, SHAPES_BUFFER_SIZE, idx);

				for(uint32_t i = 0; i < written; ++i) {
					UpdateShapeLocalPose(shapes[i], model, collider->shapes.at(idx + i));
				}

				idx += written;
			}
		}
	}

	void UpdateModel(Model * model, Collider * collider) {

	}

public:
	void Run(GameObject * gameObject);

	void operator()(GameObject * gameObject, Model * model, Collider * collider) {
		if(collider->actorRef == nullptr) {
			return;
		}

		if(model->boneData != nullptr) {
			UpdateBoneAttachedShapes(model, collider);
		}


	}
	
};

