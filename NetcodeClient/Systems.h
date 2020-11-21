#pragma once

#include "GameObject.h"
#include <Netcode/Modules.h>
#include "GraphicsEngine.h"
#include <Netcode/Input.h>
#include "PhysxHelpers.h"
#include <Netcode/MovementController.h>
#include <Netcode/Animation/Blender.h>
#include <Netcode/Animation/IK.h>
#include "Snippets.h"

class TransformSystem {
public:
	static inline void SyncFromTransformToCollider(physx::PxRigidDynamic * actor, Transform * transform) {
		physx::PxTransform actorTransform = transform->WorldToPhysX();
		actor->setKinematicTarget(actorTransform);
	}

	static inline void SyncFromColliderToTransform(physx::PxRigidDynamic * actor, Transform *parentTransform, Transform * transform) {
		physx::PxTransform pose = actor->getGlobalPose();

		transform->worldPosition = Netcode::ToFloat3(pose.p);
		transform->worldRotation = Netcode::ToFloat4(pose.q);

		if(parentTransform != nullptr) {
			Netcode::Quaternion worldRot = transform->worldRotation;
			Netcode::Vector3 worldPos = transform->worldPosition;

			Netcode::Quaternion parentWorldRot = parentTransform->worldRotation;
			Netcode::Vector3 parentWorldPos = parentTransform->worldPosition;

			Netcode::Quaternion parentInvRot = parentWorldRot.Conjugate();
			Netcode::Vector3 rotatedLocalPosition = worldPos - parentWorldPos;

			transform->position = rotatedLocalPosition.Rotate(parentInvRot);
			transform->rotation = worldRot * parentInvRot;
		} else {
			transform->position = transform->worldPosition;
			transform->rotation = transform->worldRotation;
		}
	}

	static inline void SyncTransformWithCollider(Transform * parentTransform, Transform * transform, Collider * collider) {
		if(physx::PxRigidDynamic * rigidDynamic = collider->actor->is<physx::PxRigidDynamic>(); rigidDynamic != nullptr) {
			auto rigidBodyFlags = rigidDynamic->getRigidBodyFlags();

			if(rigidBodyFlags.isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
				// sync NETCODE -> PHYSX
				SyncFromTransformToCollider(rigidDynamic, transform);
			} else {
				// sync PHYSX -> NETCODE
				SyncFromColliderToTransform(rigidDynamic, parentTransform, transform);
			}
		}
	}

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
		GameObject * parent = gameObject->Parent();
		Transform * parentTransform = (parent != nullptr) ? parent->GetComponent<Transform>() : nullptr;

		transform->worldPosition = GetWorldPosition(transform, parentTransform);
		transform->worldRotation = GetWorldRotation(transform, parentTransform);

		if(gameObject->HasComponent<Collider>()) {
			Collider * collider = gameObject->GetComponent<Collider>();

			if(collider->actor != nullptr) {
				SyncTransformWithCollider(parentTransform, transform, collider);
			}
		}
	}
};

class LightSystem {
public:
	std::vector<Netcode::Light> lights;
	GraphicsEngine * renderer;

	void Run(GameObject * gameObject);
	
	void operator()(GameObject * gameObject, Transform * transform, Netcode::Light * light) {
		/*
		TODO: calculate transformations
		*/
		lights.push_back(*light);
	}
};

class ScriptSystem {
public:
	void Run(GameObject * gameObject, Netcode::GameClock* clock);

	void operator()(GameObject * gameObject, Script * script, Netcode::GameClock * clock) {
		script->Update(clock);
	}
};

class RenderSystem {
public:
	GraphicsEngine* renderer;

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
				renderer->skinnedGbufferPass_Input.push_back(RenderItem(i, &model->perObjectData, model->boneData, model->boneDataOffset,
					gameObject->GetComponent<Animation>()->debugBoneData.get()));
			} else {
				renderer->gbufferPass_Input.push_back(RenderItem(i, &model->perObjectData, nullptr, 0, nullptr));
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

			GameObject * parent = gameObject->Parent();
			
			const Netcode::Float3 parentPos = (parent != nullptr) ?
				parent->GetComponent<Transform>()->position :
				Netcode::Float3::Zero;

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
					renderer->graphics->debug->DrawPoint(wrt.translation + parentPos, 2.0f, false);
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

		const ColliderShape & c = *(static_cast<ColliderShape *>(shape->userData));

		BoneData * bd = anim->debugBoneData.get();

		Netcode::Matrix toRoot = bd->ToRootTransform[c.boneReference];
		Netcode::Quaternion rotQ = c.localRotation;

		const Netcode::Matrix T = Netcode::TranslationMatrix(c.localPosition);
		const Netcode::Matrix R = static_cast<Netcode::Matrix>(rotQ);

		toRoot = R * T * toRoot.Transpose();

		const Netcode::Float4x4 res = toRoot;

		const Netcode::Float3 tr{
			res._41,
			res._42,
			res._43
		};

		rotQ = Netcode::DecomposeRotation(toRoot);

		const physx::PxTransform pxT{ ToPxVec3(tr), ToPxQuat(rotQ) };
		shape->setLocalPose(pxT);
	}

	void UpdateBoneAttachedShapes(Model * model, Collider * collider, Animation * anim) {
		if(auto * rigidBody = collider->actor->is<physx::PxRigidDynamic>()) {
			constexpr static uint32_t SHAPES_BUFFER_SIZE = 8;

			physx::PxShape * shapes[SHAPES_BUFFER_SIZE] = {};
			const uint32_t shapesCount = rigidBody->getNbShapes();

			uint32_t idx = 0;
			while(idx < shapesCount) {
				const uint32_t written = rigidBody->getShapes(shapes, SHAPES_BUFFER_SIZE, idx);

				for(uint32_t i = 0; i < written; i++) {
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
		if(collider->actor == nullptr) {
			return;
		}

		if(!gameObject->HasComponent<Animation>())
			return;

		Animation * anim = gameObject->GetComponent<Animation>();

		if(anim->debugBoneData != nullptr) {
			UpdateBoneAttachedShapes(model, collider, anim);
#if defined(NETCODE_DEBUG)
			// this is a VERY expensive call
			DrawDebugCollider(collider);
#endif
		}
	}
};
