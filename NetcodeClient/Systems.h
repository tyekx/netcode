#pragma once

#include "GameObject.h"
#include <Netcode/Modules.h>
#include "GraphicsEngine.h"
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
				renderer.skinnedGbufferPass_Input.push_back(RenderItem(i, &model->perObjectData, model->boneData, model->boneDataOffset,
					gameObject->GetComponent<Animation>()->debugBoneData.get()));
			} else {
				renderer.gbufferPass_Input.push_back(RenderItem(i, &model->perObjectData, nullptr, 0, nullptr));
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
					renderer->graphics->debug->DrawPoint(wrt.translation, 2.0f);
					bid = anim->bones[bid].parentId;
				}

				Netcode::Float3 startAt = Netcode::Animation::BackwardBounceCCD::GetP_c(ike, 0, anim->bones, boneTransforms);
				Netcode::Float3 p1 = Netcode::Animation::BackwardBounceCCD::GetP_e(ike, anim->bones, boneTransforms);
				
				renderer->graphics->debug->DrawLine(startAt, p1, Netcode::Float3{ 1.0f, 0.5f, 0.2f });
				renderer->graphics->debug->DrawPoint(Netcode::Float3{ p.x, p.y, p.z }, 5.0f);
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
