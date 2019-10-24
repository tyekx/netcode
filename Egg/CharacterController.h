#pragma once

#include "Asset/Model.h"
#include "AnimationController.h"
#include "ConstantBuffer.hpp"
#include "AnimationComponent.h"
#include "PsoManager.h"
#include "Mesh/MultiMesh.h"
#include "ModelComponent.h"
#include "ShaderProgram.h"
#include "PhysxSystem.h"
#include "DebugPhysx.h"

namespace Egg {

	class CharacterController {
	public:
		struct Hitbox {
			int BoneId;
			physx::PxShape * shape;
		};

		ConstantBuffer<BoneDataCb> boneDataCb;
		ConstantBuffer<PerObjectCb> cb;
		ConstantBufferVector<PerMeshCb> * meshesRef;
		std::unique_ptr<Egg::AnimationController> animCtrl;
		Egg::Asset::Model characterModel;
		Egg::Mesh::MultiMesh::P multiMesh;
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
		
		void UpdateHitboxes() {
			for(const Hitbox & hitbox : hitboxes) {
				DirectX::XMMATRIX srt = DirectX::XMLoadFloat4x4A(&boneDataCb->ToRootTransform[hitbox.BoneId]);

				DirectX::XMVECTOR scale;
				DirectX::XMVECTOR translation;
				DirectX::XMVECTOR rotation;
				DirectX::XMMatrixDecompose(&scale, &rotation, &translation, srt);

				DirectX::XMFLOAT3A translationValue;
				DirectX::XMFLOAT4A rotationValue;
				DirectX::XMStoreFloat3A(&translationValue, translation);
				DirectX::XMStoreFloat4A(&rotationValue, rotation);

				physx::PxTransform pxTransform;
				pxTransform.p = physx::PxVec3{ translationValue.x, translationValue.y, translationValue.z };
				pxTransform.q = physx::PxQuat{ rotationValue.x, rotationValue.y, rotationValue.z, rotationValue.w };

				hitbox.shape->setLocalPose(pxTransform);
			}
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
		CharacterController() : boneDataCb{}, cb{}, animCtrl{}, characterModel{}, multiMesh{}, controller{ nullptr }, onGround{ false }, speed{ 320.0f } {
			
		}

		void SetCharacterModel(Egg::Asset::Model cm) {
			std::swap(characterModel, cm);
		}

		const physx::PxExtendedVec3 & GetPosition() {
			return controller->getPosition();
		}

		void Update(float dt) {
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

			DirectX::XMStoreFloat4x4A(&cb->Model, DirectX::XMMatrixTranspose(offset));
			DirectX::XMStoreFloat4x4A(&cb->InvModel, DirectX::XMMatrixIdentity());
			cb.Upload();

			animCtrl->Animate(boneDataCb, dt);
			UpdateHitboxes();


			boneDataCb.Upload();
		}

		void Draw(ID3D12GraphicsCommandList * gcl, ConstantBuffer<PerFrameCb> & pfcb) {
			auto meshes = multiMesh->GetMeshes();
			for(auto & i : meshes) {
				auto mat = i->GetMaterial();
				mat->ApplyPipelineState(gcl);
				mat->BindConstantBuffer(gcl, pfcb);
				mat->BindConstantBuffer(gcl, boneDataCb);
				mat->BindConstantBuffer(gcl, cb);
				mat->BindConstantBuffer(gcl, PerMeshCb::id, meshesRef->TranslateAddr(i->GetMeshData()));

				auto geom = i->GetGeometry();
				geom->Draw(gcl);
			}
		}

		void CreateResources(ID3D12Device * device, Egg::PsoManager * psoMan, Egg::DebugPhysx* dbPx, physx::PxControllerManager* ctrlManager, ConstantBufferVector<PerMeshCb> & meshesCb) {
			boneDataCb.CreateResources(device);
			cb.CreateResources(device);
			meshesRef = &meshesCb;

			com_ptr<ID3DBlob> avatarVS = Egg::ShaderProgram::LoadCso(L"AvatarVS.cso");
			com_ptr<ID3DBlob> avatarPS = Egg::ShaderProgram::LoadCso(L"AvatarPS.cso");
			com_ptr<ID3D12RootSignature> rootSig = Egg::ShaderProgram::LoadRootSignature(device, avatarVS.Get());

			animCtrl.reset(new Egg::AnimationController{ characterModel.animations, characterModel.animationsLength,
														 characterModel.bones, characterModel.bonesLength });

			multiMesh = Egg::Mesh::MultiMesh::Create();

			Egg::PipelineState::P pso = Egg::PipelineState::Create();
			pso->SetRootSignature(rootSig);
			pso->SetVertexShader(avatarVS);
			pso->SetPixelShader(avatarPS);
			pso->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
			pso->SetDSVFormat(DXGI_FORMAT_D32_FLOAT);

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


			for(unsigned int i = 0; i < characterModel.meshesLength; ++i) {

				Egg::Mesh::Geometry::P geom = Egg::Mesh::IndexedGeometry::Create(device,
																				 characterModel.meshes[i].vertices, characterModel.meshes[i].verticesLength, characterModel.meshes[i].vertexSize,
																				 characterModel.meshes[i].indices, characterModel.meshes[i].indicesLength * (UINT32)sizeof(unsigned int));
				geom->SetVertexType(characterModel.meshes[i].vertexType);
				Egg::Material::P mat = Egg::Material::Create(psoMan, geom, pso);

				mat->ConstantBufferSlot(0, PerMeshCb::id);
				mat->ConstantBufferSlot(1, PerObjectCb::id);
				mat->ConstantBufferSlot(2, BoneDataCb::id);
				mat->ConstantBufferSlot(3, PerFrameCb::id);

				PerMeshCb * perMeshData = meshesCb.Next();
				perMeshData->diffuseColor = DirectX::XMFLOAT4A{ characterModel.materials[i].diffuseColor.x, characterModel.materials[i].diffuseColor.y, characterModel.materials[i].diffuseColor.z, 1.0f };
				perMeshData->fresnelR0 = DirectX::XMFLOAT3{ 0.05f, 0.05f, 0.05f };
				perMeshData->shininess = 2.0f;

				multiMesh->Add(geom, mat, perMeshData);
			}
		}



	};

}
