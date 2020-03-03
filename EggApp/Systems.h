#pragma once

#include "GameObject.h"
#include <Egg/Modules.h>
#include "GraphicsEngine.h"
#include "UIObject.h"
#include <Egg/Input.h>
#include "PhysxHelpers.h"

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
	physx::PxPhysics * px;
	physx::PxScene * pxScene;
	physx::PxMaterial * dummyMaterial;
	DirectX::XMINT2 screenSize;
	PerFrameData * perFrameData;
	std::list<Button *> raycastHits;
	bool lmbHeld;
public:
	GraphicsEngine * gEngine;

	void SetScreenSize(const DirectX::XMUINT2 & dim) {
		screenSize = DirectX::XMINT2{ static_cast<int32_t>(dim.x), static_cast<int32_t>(dim.y) };
	}

	void CreateResources(physx::PxScene * pxS, PerFrameData* pfd) {
		perFrameData = pfd;
		pxScene = pxS;
		px = &pxS->getPhysics();
		dummyMaterial = px->createMaterial(0.5f, 0.5f, 0.5f);
	}

	void Raycast() {
		float lmb = Egg::Input::GetAxis("Fire");
		DirectX::XMINT2 mousePos = Egg::Input::GetMousePos();

		bool isClicked = false;

		if(lmb > 0.0f && !lmbHeld) {
			lmbHeld = true;
			isClicked = true;
		}

		if(lmb == 0.0f && lmbHeld) {
			lmbHeld = false;
		}

		DirectX::XMFLOAT4 ndcMousePos{
			static_cast<float>(mousePos.x),
			static_cast<float>(mousePos.y),
			0.0f,
			1.0f
		};

		DirectX::XMVECTOR ndcMousePosV = DirectX::XMLoadFloat4(&ndcMousePos);
		DirectX::XMVECTOR rayDirVector = DirectX::g_XMIdentityR2;

		DirectX::XMFLOAT3 raycastRayDir;
		DirectX::XMFLOAT3 raycastRayStart;

		DirectX::XMStoreFloat3(&raycastRayStart, ndcMousePosV);
		DirectX::XMStoreFloat3(&raycastRayDir, rayDirVector);

		physx::PxVec3 pxRayStart = ToPxVec3(raycastRayStart);
		physx::PxVec3 pxRayDir = ToPxVec3(raycastRayDir);

		physx::PxQueryFilterData filterData;
		filterData.data.word0 = PHYSX_COLLIDER_TYPE_UI;

		physx::PxRaycastBuffer outRaycastResult;
		raycastHits.clear();

		if(pxScene->raycast(pxRayStart, pxRayDir, 1.0f, outRaycastResult)) {
			uint32_t numHits = outRaycastResult.getNbAnyHits();
			for(uint32_t i = 0; i < numHits; ++i) {
				const auto & hit = outRaycastResult.getAnyHit(i);
				UIObject * obj = static_cast<UIObject *>(hit.actor->userData);
				Button * btn = obj->GetComponent<Button>();

				raycastHits.push_back(btn);

				if(isClicked) {
					btn->onClick();
				}
			}
		}

		gEngine->uiPass_viewProjInv = perFrameData->ViewProj;
	}

	void Run(UIObject * object);

	void operator()(UIObject * uiObject, Transform* transform, UIElement * uiElement, Sprite* sprite, Button* button) {
		// handle spawning
		if(uiObject->IsSpawnable()) {
			uiObject->SetSpawnableFlag(false);
			if(button->pxActor == nullptr) {
				physx::PxVec3 pos = ToPxVec3(transform->worldPosition);
				physx::PxQuat rot = ToPxQuat(transform->worldRotation);

				physx::PxRigidDynamic * actor = px->createRigidDynamic(physx::PxTransform(pos, rot));
				actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
				physx::PxBoxGeometry boxGeom{ uiElement->width / 2.0f, uiElement->height / 2.0f, 0.01f };
				physx::PxShape * boxShape = px->createShape(boxGeom, *dummyMaterial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eSCENE_QUERY_SHAPE);
				physx::PxFilterData filterData;
				filterData.word0 = PHYSX_COLLIDER_TYPE_UI;
				boxShape->setSimulationFilterData(filterData);
				boxShape->setQueryFilterData(filterData);
				physx::PxTransform localPose(physx::PxIdentity);
				localPose.p = physx::PxVec3(uiElement->width / 2.0f, uiElement->height / 2.0f, 0.0f);
				boxShape->setLocalPose(localPose);
				physx::PxVec3 gPos = ToPxVec3(transform->position);
				physx::PxTransform globalPose(physx::PxIdentity);
				globalPose.p = gPos;
				actor->setGlobalPose(globalPose);
				actor->attachShape(*boxShape);
				actor->userData = uiObject;
				pxScene->addActor(*actor);
			}
		}

		bool found = false;

		gEngine->uiPass_Input.Produced(UIRenderItem{ sprite->texture, nullptr, nullptr, DirectX::XMFLOAT4{  DirectX::g_XMZero }, DirectX::XMFLOAT2{ transform->position.x, transform->position.y }, sprite->textureSize });

		// handle mouseovers
		for(auto it = raycastHits.begin(); it != raycastHits.end(); ++it) {
			if(button == *it) {
				if(!button->isMouseOver) {
					button->isMouseOver = true;
					button->onMouseEnter();
				}

				found = true;
				raycastHits.erase(it);
				break;
			}
		}

		if(!found) {
			if(button->isMouseOver) {
				button->isMouseOver = false;
				button->onMouseLeave();
			}
		}
	}
};

class UITextSystem {
public:
	GraphicsEngine * gEngine;

	void Run(UIObject * object);

	void operator()(UIObject * uiObject, Transform * transform, UIElement * uiElement, Text * text) {
		gEngine->uiPass_Input.Produced(UIRenderItem{
											nullptr,
											text->font,
											text->text.c_str(),
											text->color,
											DirectX::XMFLOAT2 { transform->position.x, transform->position.y },
											DirectX::XMUINT2 { 0, 0 }
									   });
	}
};
