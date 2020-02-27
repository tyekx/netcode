#pragma once

#include "GameObject.h"
#include <Egg/Modules.h>
#include "GraphicsEngine.h"
#include "UIObject.h"
#include <Egg/Input.h>

class TransformSystem {

	static inline DirectX::XMVECTOR GetWorldRotation(Transform * transform, Transform * parentTransform) {
		DirectX::XMVECTOR worldRotation = DirectX::XMLoadFloat4(&transform->rotation);
		if(parentTransform != nullptr) {
			DirectX::XMVECTOR parentWorldRotation = DirectX::XMLoadFloat4(&parentTransform->worldRotation);

			worldRotation = DirectX::XMQuaternionMultiply(worldRotation, parentWorldRotation);
		}

		return worldRotation;
	}

	static inline DirectX::XMVECTOR GetWorldPosition(Transform * transform, Transform * parentTransform) {
		DirectX::XMVECTOR worldPosition = DirectX::XMLoadFloat3(&transform->position);

		if(parentTransform != nullptr) {
			DirectX::XMVECTOR parentWorldPosition = DirectX::XMLoadFloat3(&parentTransform->worldPosition);

			worldPosition = DirectX::XMVectorAdd(worldPosition, parentWorldPosition);
		}

		return worldPosition;
	}
public:

	void Run(GameObject * gameObject);

	void operator()(GameObject * gameObject, Transform * transform) {
		DirectX::XMVECTOR worldPos;
		DirectX::XMVECTOR worldRot;

		GameObject * parent = gameObject->Parent();

		if(parent != nullptr) {
			Transform * parentTransform = parent->GetComponent<Transform>();
			worldPos = GetWorldPosition(transform, parentTransform);
			worldRot = GetWorldRotation(transform, parentTransform);
		} else {
			worldPos = GetWorldPosition(transform, nullptr);
			worldRot = GetWorldRotation(transform, nullptr);
		}

		DirectX::XMStoreFloat3(&transform->worldPosition, worldPos);
		DirectX::XMStoreFloat4(&transform->worldRotation, worldRot);
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
	GraphicsEngine renderer;

	void CreatePermanentResources(Egg::Module::IGraphicsModule * graphics) {
		renderer.CreatePermanentResources(graphics);
	}

	void Run(GameObject * gameObject);

	void operator()(GameObject * gameObject, Transform * transform, Model * model) {
		DirectX::XMVECTOR worldPos = DirectX::XMLoadFloat3(&transform->position);
		DirectX::XMVECTOR worldRot = DirectX::XMLoadFloat4(&transform->rotation);
		DirectX::XMVECTOR scaleVector = DirectX::XMLoadFloat3(&transform->scale);
		DirectX::XMMATRIX modelMat = DirectX::XMMatrixAffineTransformation(scaleVector, DirectX::XMQuaternionIdentity(), worldRot, worldPos);
		DirectX::XMVECTOR determinant = DirectX::XMMatrixDeterminant(modelMat);
		DirectX::XMMATRIX invModelMat = DirectX::XMMatrixInverse(&determinant, modelMat);
		DirectX::XMStoreFloat4x4A(&model->perObjectData.Model, DirectX::XMMatrixTranspose(modelMat));
		DirectX::XMStoreFloat4x4A(&model->perObjectData.InvModel, DirectX::XMMatrixTranspose(invModelMat));

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
	void UpdateShapeLocalPose(physx::PxShape* shape, Model* model) {
		if(shape->userData == nullptr) {
			return;
		}

		const ColliderShape & c = *(reinterpret_cast<ColliderShape *>(shape->userData));

		DirectX::XMMATRIX toRoot = DirectX::XMLoadFloat4x4A(&model->boneData->ToRootTransform[c.boneReference]);
		DirectX::XMVECTOR rotQ = DirectX::XMLoadFloat4(&c.localRotation);

		DirectX::FXMMATRIX T = DirectX::XMMatrixTranslation(c.localPosition.x, c.localPosition.y, c.localPosition.z);
		DirectX::FXMMATRIX R = DirectX::XMMatrixRotationQuaternion(rotQ);

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
					UpdateShapeLocalPose(shapes[i], model);
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


class UISystem {
	DirectX::XMINT2 screenSize;
public:
	PerFrameData * perFrameData;

	void SetScreenSize(const DirectX::XMUINT2 & dim) {
		screenSize = DirectX::XMINT2{ static_cast<int32_t>(dim.x), static_cast<int32_t>(dim.y) };
	}

	void Run(UIObject * object);

	void Update() {
		float lmb = Egg::Input::GetAxis("Fire");

		DirectX::XMINT2 mousePos = Egg::Input::GetMousePos();

		DirectX::XMFLOAT4 ndcMousePos{
			static_cast<float>(mousePos.x) / static_cast<float>(screenSize.x),
			static_cast<float>(mousePos.y) / static_cast<float>(screenSize.y),
			0.0f,
			1.0f
		};

		DirectX::XMVECTOR ndcMousePosV = DirectX::XMLoadFloat4(&ndcMousePos);
		DirectX::XMMATRIX viewProjInvV = DirectX::XMLoadFloat4x4A(&perFrameData->ViewProjInv);
		DirectX::XMMATRIX rayDirV = DirectX::XMLoadFloat4x4A(&perFrameData->RayDir);

		DirectX::XMVECTOR modelSpaceMousePos = DirectX::XMVector4Transform(ndcMousePosV, viewProjInvV);
		DirectX::XMVECTOR rayDirVector = DirectX::XMVector4Transform(ndcMousePosV, rayDirV);

		
	}

	void operator()(UIObject * uiObject, Transform* transform, Button* button) {


	}
};
