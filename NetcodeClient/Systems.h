#pragma once

#include "GameObject.h"
#include <Netcode/Modules.h>
#include "GraphicsEngine.h"
#include "UIObject.h"
#include <Netcode/Input.h>
#include "PhysxHelpers.h"
#include <Netcode/MovementController.h>
#include <Netcode/Animation/Blender.h>
#include <Netcode/Animation/IK.h>

class TransformSystem {
public:
	static inline Netcode::Quaternion GetWorldRotation(Transform * transform, Transform * parentTransform) {
		Netcode::Quaternion worldRotation{ transform->rotation };
		if(parentTransform != nullptr) {
			worldRotation = worldRotation * parentTransform->worldRotation;
		}

		return worldRotation;
	}

	static inline Netcode::Vector3 GetWorldPosition(Transform * transform, Transform * parentTransform) {
		Netcode::Vector3 localPosition{ transform->position };

		if(parentTransform != nullptr) {
			Netcode::Quaternion parentWorldRotation = parentTransform->worldRotation;
			Netcode::Vector3 parentWorldPosition = parentTransform->worldPosition;

			return localPosition.Rotate(parentWorldRotation) + parentWorldPosition;
		}

		return localPosition;
	}

	void Run(GameObject * gameObject);

	void operator()(GameObject * gameObject, Transform * transform) {
		Netcode::Vector3 worldPos;
		Netcode::Quaternion worldRot;

		GameObject * parent = gameObject->Parent();

		if(parent != nullptr) {
			Transform * parentTransform = parent->GetComponent<Transform>();
			worldPos = GetWorldPosition(transform, parentTransform);
			worldRot = GetWorldRotation(transform, parentTransform);
		} else {
			worldPos = GetWorldPosition(transform, nullptr);
			worldRot = GetWorldRotation(transform, nullptr);
		}

		transform->worldPosition = worldPos;
		transform->worldRotation = worldRot;
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

	void CreatePermanentResources(Netcode::Module::IGraphicsModule * graphics) {
		renderer.CreatePermanentResources(graphics);
	}

	void Run(GameObject * gameObject);

	void operator()(GameObject * gameObject, Transform * transform, Model * model) {
		Netcode::Vector3 worldPos = transform->worldPosition;
		Netcode::Vector3 scaleV = transform->scale;
		Netcode::Quaternion worldRot = transform->worldRotation;

		Netcode::Matrix modelMat = Netcode::AffineTransformation(scaleV, worldRot, worldPos);
		Netcode::Matrix invModelMat = modelMat.Invert();

		model->perObjectData.Model = modelMat.Transpose();
		model->perObjectData.InvModel = invModelMat.Transpose();


		bool hasDebugData = false;
		if(gameObject->HasComponent<Animation>()) {
			hasDebugData = gameObject->GetComponent<Animation>()->debugBoneData != nullptr;
		}

		for(const auto & i : model->meshes) {

			if(model->boneData != nullptr || hasDebugData) {
				renderer.skinnedGbufferPass_Input.Produced(RenderItem(i, &model->perObjectData, model->boneData, model->boneDataOffset,
					gameObject->GetComponent<Animation>()->debugBoneData.get()));
			} else {
				renderer.gbufferPass_Input.Produced(RenderItem(i, &model->perObjectData, nullptr, 0, nullptr));
			}
		}
	}
};

class AnimationSystem {
	Netcode::MovementController * movementController;
public:
	void Run(GameObject * gameObject, float dt);

	GraphicsEngine * renderer;

	void SetMovementController(Netcode::MovementController * movCtrl) {
		movementController = movCtrl;
	}

	void operator()(GameObject * gameObject, Model * model, Animation* anim, float dt) {
		if(anim->controller != nullptr && movementController != nullptr) {
			movementController->Update();

			anim->blackboard->Update(dt);

			anim->blender->UpdatePlan(anim->clips, anim->blackboard->GetActiveStates());

			if(anim->debugBoneData == nullptr) {
				anim->debugBoneData = std::make_unique<BoneData>();
			}

			anim->blender->Blend(anim->clips);

			BoneData * res = anim->debugBoneData.get();

			auto boneTransforms = anim->blender->GetBoneTransforms();

			for(const Netcode::Animation::IKEffector & effector : anim->effectors) {
				Netcode::Animation::FABRIK::Run(effector, anim->bones, boneTransforms);
			}

			Netcode::Quaternion h = anim->headRotation;
			for(uint32_t i = 0; i < 5; ++i) {
				h = h * boneTransforms[i].rotation.Conjugate();
			}

			boneTransforms[5].rotation = h;

			for(const Netcode::Animation::IKEffector & effector : anim->effectors) {
				const auto & p = effector.position;
				Netcode::Animation::IKEffector ike = effector;
				ike.chainLength += 1;

				int32_t bid = ike.parentId;
				while(bid >= 0) {
					auto wrt = Netcode::Animation::BackwardBounceCCD::GetWorldRT(bid, anim->bones, boneTransforms);
					//renderer->DrawDebugPoint(wrt.translation, 2.0f);
					bid = anim->bones[bid].parentId;
				}

				Netcode::Float3 startAt = Netcode::Animation::BackwardBounceCCD::GetP_c(ike, 0, anim->bones, boneTransforms);
				Netcode::Float3 p1 = Netcode::Animation::BackwardBounceCCD::GetP_e(ike, anim->bones, boneTransforms);
				
				//renderer->DrawDebugVector(startAt, p1, Netcode::Float3{ 1.0f, 0.5f, 0.2f });
				//renderer->DrawDebugPoint(Netcode::Float3{ p.x, p.y, p.z }, 10.0f);
			}

			anim->blender->UpdateMatrices(anim->bones, res->ToRootTransform, res->BindTransform);

			/*
			

			anim->controller->Animate(blender->GetPlan());
			int32_t id = anim->controller->GetId();

			auto animSet = anim->controller->GetAnimationSet();

			model->boneData = animSet->GetResultsView();
			model->boneDataOffset = id;

			if(id == 0) {
				renderer->skinningPass_Input.Produced(animSet);
			}*/
		}
	}
};

class PhysXSystem {
	void UpdateShapeLocalPose(physx::PxShape* shape, Model* model, Animation * anim) {
		if(shape->userData == nullptr) {
			return;
		}

		const ColliderShape & c = *(reinterpret_cast<ColliderShape *>(shape->userData));

		BoneData * bd = anim->controller->GetAnimationSet()->GetData() + model->boneDataOffset;

		Netcode::Matrix toRoot = bd->ToRootTransform[c.boneReference];
		Netcode::Quaternion rotQ = c.localRotation;

		Netcode::Matrix T = Netcode::TranslationMatrix(c.localPosition);
		Netcode::Matrix R = (Netcode::Matrix)rotQ;

		toRoot = R * T * toRoot.Transpose();

		Netcode::Float4x4 res = toRoot;

		Netcode::Float3 tr{
			res._41,
			res._42,
			res._43
		};

		rotQ = Netcode::DecomposeRotation(toRoot);

		physx::PxTransform pxT{ ToPxVec3(tr), ToPxQuat(rotQ) };
		shape->setLocalPose(pxT);
	}

	void UpdateBoneAttachedShapes(Model * model, Collider * collider, Animation * anim) {
		if(auto * rigidBody = collider->actorRef->is<physx::PxRigidDynamic>()) {
			constexpr static uint32_t SHAPES_BUFFER_SIZE = 8;

			physx::PxShape * shapes[SHAPES_BUFFER_SIZE] = {};
			uint32_t shapesCount = rigidBody->getNbShapes();

			uint32_t idx = 0;
			while(idx < shapesCount) {
				uint32_t written = rigidBody->getShapes(shapes, SHAPES_BUFFER_SIZE, idx);

				for(uint32_t i = 0; i < written; ++i) {
					UpdateShapeLocalPose(shapes[i], model, anim);
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
			if(gameObject->HasComponent<Animation>()) {
				UpdateBoneAttachedShapes(model, collider, gameObject->GetComponent<Animation>());
			}
		}
	}
};


class UISystem {
	physx::PxPhysics * px;
	physx::PxScene * pxScene;
	physx::PxMaterial * dummyMaterial;
	Netcode::Int2 screenSize;
	PerFrameData * perFrameData;
	std::list<Button *> raycastHits;
	bool lmbHeld;
public:
	GraphicsEngine * gEngine;

	void SetScreenSize(const Netcode::UInt2 & dim) {
		screenSize = Netcode::Int2{ static_cast<int32_t>(dim.x), static_cast<int32_t>(dim.y) };
	}

	void CreateResources(physx::PxScene * pxS, PerFrameData* pfd) {
		perFrameData = pfd;
		pxScene = pxS;
		px = &pxS->getPhysics();
		dummyMaterial = px->createMaterial(0.5f, 0.5f, 0.5f);
	}

	void Raycast() {
		float lmb = Netcode::Input::GetAxis("Fire");
		Netcode::Int2 mousePos = Netcode::Input::GetMousePos();

		bool isClicked = false;

		if(lmb > 0.0f && !lmbHeld) {
			lmbHeld = true;
			isClicked = true;
		}

		if(lmb == 0.0f && lmbHeld) {
			lmbHeld = false;
		}

		Netcode::Float3 raycastRayStart{
			static_cast<float>(mousePos.x),
			static_cast<float>(mousePos.y),
			0.0f
		};

		Netcode::Float3 raycastRayDir{
			0.0f, 0.0f, 1.0f
		};

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
					if(btn->onClick != nullptr) {
						btn->onClick();
					}
				}
			}
		}

		gEngine->uiPass_viewProjInv = perFrameData->ViewProj;
	}

	void Run(UIObject * object);

	void operator()(UIObject * uiObject, Transform* transform, UIElement * uiElement) {
		Button * button = nullptr;

		if(uiObject->HasComponent<Button>()) {
			button = uiObject->GetComponent<Button>();
			if(uiObject->IsActive()) {
				if(button->pxActor != nullptr) {
					if(!button->isSpawned) {
						pxScene->addActor(*button->pxActor);
						button->isSpawned = true;
					}

					physx::PxVec3 pos = ToPxVec3(transform->worldPosition);
					physx::PxQuat rot = ToPxQuat(transform->worldRotation);

					static_cast<physx::PxRigidDynamic *>(button->pxActor)->setGlobalPose(physx::PxTransform{ pos, rot });
				}
			} else {
				if(button->pxActor != nullptr) {
					if(button->isSpawned) {
						pxScene->removeActor(*button->pxActor);
						button->isSpawned = false;
					}
				}
			}
		}

		// handle spawning
		if(uiObject->IsSpawnable()) {
			uiObject->SetSpawnableFlag(false);

			if(button != nullptr) {
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
					actor->attachShape(*boxShape);
					actor->userData = uiObject;
					pxScene->addActor(*actor);
					button->pxActor = actor;
					button->isSpawned = true;
				}
			}
		}

		if(button != nullptr && uiObject->IsActive()) {
			bool found = false;

			// handle mouseovers
			for(auto it = raycastHits.begin(); it != raycastHits.end(); ++it) {
				if(button == *it) {
					if(!button->isMouseOver) {
						button->isMouseOver = true;
						if(button->onMouseEnter != nullptr) {
							button->onMouseEnter();
						}
					}

					found = true;
					raycastHits.erase(it);
					break;
				}
			}

			if(!found) {
				if(button->isMouseOver) {
					button->isMouseOver = false;
					if(button->onMouseLeave != nullptr) {
						button->onMouseLeave();
					}
				}
			}
		}
	}
};

class UITransformSystem {
public:
	void Run(UIObject * object);

	void operator()(UIObject * uiObject, Transform * transform) {
		UIObject * parent = uiObject->Parent();
		bool isParentActive = true;
		Transform * parentTransform = nullptr;

		if(parent != nullptr) {
			parentTransform = parent->GetComponent<Transform>();
			isParentActive = parent->IsActive();
		}

		uiObject->IsActive(uiObject->GetActivityFlag() && isParentActive);

		transform->worldPosition = TransformSystem::GetWorldPosition(transform, parentTransform);
		transform->worldRotation = TransformSystem::GetWorldRotation(transform, parentTransform);
	}
};

class UISpriteSystem {
public:
	GraphicsEngine * gEngine;
	
	void Run(UIObject * uiObject);

	void operator()(UIObject * uiObject, Transform * transform, UIElement * uiElement, Sprite * sprite) {
		Netcode::Float4 clr = sprite->diffuseColor;

		if(!uiObject->IsActive()) {
			return;
		}

		if(uiObject->HasComponent<Button>()) {
			const auto & children = uiObject->GetChildren();
			Button * button = uiObject->GetComponent<Button>();

			bool isMouseOver = button->isMouseOver;

			if(children.size() == 1 && (*children.begin())->HasComponent<TextBox>()) {
				TextBox * tb = (*children.begin())->GetComponent<TextBox>();

				if(tb->id == TextBox::selectedId) {
					isMouseOver = true;
				}
			}

			if(isMouseOver) {
				clr = sprite->hoverColor;
			}

		}

		gEngine->uiPass_Input.Produced(UISpriteRenderItem(
			sprite->texture,
			sprite->textureSize,
			Netcode::Float2{ static_cast<float>(uiElement->width), static_cast<float>(uiElement->height) },
			clr,
			Netcode::Float2{ transform->worldPosition.x, transform->worldPosition.y },
			Netcode::Float2{ uiElement->origin },
			uiElement->rotationZ
		));
	}
};

class UIAnimSystem {
public:
	void Run(UIObject * object, float dt);

	void operator()(UIObject * uiObject, UIScript * uiScript, float dt) {
		if(uiScript->onUpdate) {
			uiScript->onUpdate(uiObject, dt);
		}
	}
};

class UITextSystem {
public:
	GraphicsEngine * gEngine;

	void Run(UIObject * object);

	void operator()(UIObject * uiObject, Transform * transform, UIElement * uiElement, Text * text) {
		if(uiObject->IsActive()) {

			std::wstring displayString = text->text;
			Netcode::Float4 displayColor = text->color;

			if(uiObject->HasComponent<TextBox>()) {
				TextBox * tb = uiObject->GetComponent<TextBox>();
				
				if(text->text.empty()) {
					displayString = tb->placeholder;
					displayColor = tb->placeholderColor;
				} else if(tb->isPassword) {
					displayString = std::wstring(text->text.size(), L'*');
				}
			}

			gEngine->uiPass_Input.Produced(
				UITextRenderItem(
					text->font,
					std::move(displayString),
					Netcode::Float2 { transform->worldPosition.x, transform->worldPosition.y },
					displayColor
				));
		}
	}
};
