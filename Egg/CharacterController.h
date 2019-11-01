#pragma once

#include "Asset/Model.h"
#include "AnimationController.h"
#include "ConstantBuffer.hpp"
#include "AnimationComponent.h"
#include "ModelComponent.h"
#include "PhysxSystem.h"
#include "DebugPhysx.h"
#include "Multi.h"
/*
namespace Egg {

	class CharacterController {
	public:
		struct Hitbox {
			int BoneId;
			physx::PxShape * shape;
		};

		Multi multiMesh;

		Egg::Asset::Model characterModel;
		physx::PxController * controller;
		bool onGround;
		float speed;
		DirectX::XMFLOAT3A airVelocity;

		std::vector<Hitbox> hitboxes;

		int FindBone(const std::string & name) {
			for(int i = 0; i < (int)characterModel.bonesLength; ++i) {
				if(name == characterModel.bones[i].name) {
					return i;
				}
			}
			return -1;
		}

		void AttachHitboxes() {

			auto* pxScene = controller->getScene();
			auto& pxPhysics = controller->getScene()->getPhysics();

			physx::PxCapsuleGeometry bodyColliderGeometry{ 15.0f, 30.0f };
			physx::PxMaterial * dummyMaterial = pxPhysics.createMaterial(0.0f, 0.0f, 0.0f);
			physx::PxShapeFlags flags = physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE;
			physx::PxShape * shape = pxPhysics.createShape(bodyColliderGeometry, *dummyMaterial, true, flags);

			hitboxes.push_back({ 26, shape });

			controller->getActor()->attachShape(*shape);

		}

	public:
		CharacterController() : animCtrl{}, characterModel{}, controller{ nullptr }, onGround{ false }, speed{ 320.0f } {
			
		}

		void SetCharacterModel(Egg::Asset::Model cm) {
			std::swap(characterModel, cm);
		}

		const physx::PxExtendedVec3 & GetPosition() {
			return controller->getPosition();
		}

		void Update(float dt) {
			return;
		float vertical = Egg::Input::GetAxis("Vertical");
			float horizontal = Egg::Input::GetAxis("Horizontal");
			float jump = Egg::Input::GetAxis("Jump");

			DirectX::XMFLOAT3A movementComponents{ horizontal, 0.0f, vertical };

			if(onGround && jump > 0.0f) {
				airVelocity.y = 500.0f;
				onGround = false;
				animCtrl->StartJump();
			}

			if(!onGround) {
				airVelocity.y -= 981.0f * dt; // 9.81 m/s^2 = 981 cm/s^2
			}


			DirectX::XMVECTOR dir = DirectX::XMLoadFloat3A(&movementComponents);
			dir = DirectX::XMVector3Normalize(dir);
			dir = DirectX::XMVectorScale(dir, speed * dt);

			DirectX::XMFLOAT3A dirResult;
			DirectX::XMStoreFloat3A(&dirResult, dir);
			dirResult.y = airVelocity.y * dt;

			physx::PxControllerCollisionFlags result = controller->move(physx::PxVec3(dirResult.x, dirResult.y, dirResult.z), 0.0f, dt, physx::PxControllerFilters{});

			if(!onGround && result.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN)) {
				onGround = true;
				animCtrl->EndJump();
			}

			auto pxV3 = controller->getPosition();
			DirectX::XMMATRIX Tr = DirectX::XMMatrixTranslation((float)pxV3.x, (float)pxV3.y, (float)pxV3.z);

			DirectX::XMMATRIX offset = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0.0f, -100.0f, 0.0f), Tr);
		}

		void Draw(ID3D12GraphicsCommandList * gcl, ConstantBuffer<PerFrameCb> & pfcb) {
			//auto meshes = multiMesh->GetMeshes();

			Mesh * meshes = multiMesh.meshes.get();
			Material * materials = multiMesh.material.get();
			
			for(UINT i = 0; i < multiMesh.length; ++i) {
				Mesh * mesh = meshes + i;
				Material * mat = materials + i;
				mat->SetPipelineState(gcl);
				//mat->BindConstantBuffer(gcl, PerMeshCb::id, );

				//auto geom = i->GetGeometry();
				//geom->Draw(gcl);
			}
		}

		void CreateResources(ID3D12Device * device, Egg::DebugPhysx* dbPx, physx::PxControllerManager* ctrlManager, ConstantBufferVector<PerMeshCb> & meshesCb) {


			animCtrl.reset(new Egg::AnimationController{ characterModel.animations, characterModel.animationsLength,
														 characterModel.bones, characterModel.bonesLength });


			physx::PxCapsuleControllerDesc cd;
			cd.behaviorCallback = NULL;
			cd.climbingMode = physx::PxCapsuleClimbingMode::eEASY;
			cd.contactOffset = 0.1f;
			cd.density = 10.0f;
			cd.invisibleWallHeight = 0.0f;
			cd.material = ctrlManager->getScene().getPhysics().createMaterial(0.5f, 0.6f, 0.6f);
			cd.position = physx::PxExtendedVec3{ 0.0f, 200.0f, 0.0f };
			cd.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
			cd.registerDeletionListener = true;
			cd.reportCallback = NULL;
			cd.scaleCoeff = 0.8f;
			cd.slopeLimit = 0.7071f;
			cd.stepOffset = 5.0f;
			cd.upDirection = physx::PxVec3{ 0.0f, 1.0f, 0.0f };
			cd.volumeGrowth = 1.5f;
			cd.height = 80.0f;
			cd.radius = 60.0f;
			
			controller = ctrlManager->createController(cd);

			//AttachHitboxes();

			//dbPx->AddActor(actor);
			dbPx->AddActor(controller->getActor());
		}



	};

}

			*/
