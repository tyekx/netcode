#pragma once

#include "Asset/Model.h"
#include "AnimationController.h"
#include "ConstantBuffer.hpp"
#include "AnimationComponent.h"
#include "PsoManager.h"
#include "Mesh/MultiMesh.h"
#include "ModelComponent.h"
#include "Shader.h"
#include "PhysxSystem.h"
#include "DebugPhysx.h"

namespace Egg {

	class TestCallback : physx::PxSimulationEventCallback {
		// Inherited via PxSimulationEventCallback

		void onConstraintBreak(physx::PxConstraintInfo * constraints, physx::PxU32 count) override
		{
		}

		void onWake(physx::PxActor ** actors, physx::PxU32 count) override
		{
		}

		void  onSleep(physx::PxActor ** actors, physx::PxU32 count) override
		{
		}

		void onContact(const physx::PxContactPairHeader & pairHeader, const physx::PxContactPair * pairs, physx::PxU32 nbPairs) override
		{
		}

		void onTrigger(physx::PxTriggerPair * pairs, physx::PxU32 count) override
		{

		}

		void onAdvance(const physx::PxRigidBody * const * bodyBuffer, const physx::PxTransform * poseBuffer, const physx::PxU32 count) override 
		{
		}

	};

	class CharacterController {

		struct Hitbox {
			int BoneId;
			physx::PxShape * shape;
		};

		ConstantBuffer<BoneDataCb> boneDataCb;
		ConstantBuffer<PerObjectCb> cb;
		std::unique_ptr<Egg::AnimationController> animCtrl;
		Egg::Asset::Model characterModel;
		Egg::Mesh::MultiMesh::P multiMesh;
		physx::PxController * controller;
		bool onGround;
		float speed;

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

			DirectX::XMFLOAT3A movementComponents{ horizontal, 0.0f, vertical };

			if(!onGround) {
				movementComponents.y = -9.81f;
			}

			DirectX::XMVECTOR dir = DirectX::XMLoadFloat3A(&movementComponents);
			dir = DirectX::XMVector3Normalize(dir);
			dir = DirectX::XMVectorScale(dir, speed * dt);

			DirectX::XMFLOAT3A dirResult;
			DirectX::XMStoreFloat3A(&dirResult, dir);

			physx::PxControllerCollisionFlags result = controller->move(physx::PxVec3(dirResult.x, dirResult.y, dirResult.z), 0.0f, dt, physx::PxControllerFilters{});

			if(result.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN)) {
				onGround = true;
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

				auto geom = i->GetGeometry();
				geom->Draw(gcl);
			}
		}

		void CreateResources(ID3D12Device * device, Egg::PsoManager * psoMan, Egg::DebugPhysx* dbPx, physx::PxControllerManager* ctrlManager) {
			boneDataCb.CreateResources(device);
			cb.CreateResources(device);

			com_ptr<ID3DBlob> avatarVS = Egg::Shader::LoadCso(L"AvatarVS.cso");
			com_ptr<ID3DBlob> avatarPS = Egg::Shader::LoadCso(L"AvatarPS.cso");
			com_ptr<ID3D12RootSignature> rootSig = Egg::Shader::LoadRootSignature(device, avatarVS.Get());

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

			AttachHitboxes();

			//dbPx->AddActor(actor);
			dbPx->AddActor(controller->getActor());


			for(unsigned int i = 0; i < characterModel.meshesLength; ++i) {

				Egg::Mesh::Geometry::P geom = Egg::Mesh::IndexedGeometry::Create(device,
																				 characterModel.meshes[i].vertices, characterModel.meshes[i].verticesLength, characterModel.meshes[i].vertexSize,
																				 characterModel.meshes[i].indices, characterModel.meshes[i].indicesLength * (UINT32)sizeof(unsigned int));
				geom->SetVertexType(characterModel.meshes[i].vertexType);
				Egg::Material::P mat = Egg::Material::Create(psoMan, geom, pso);

				mat->ConstantBufferSlot(0, PerFrameCb::id);
				mat->ConstantBufferSlot(1, PerObjectCb::id);
				mat->ConstantBufferSlot(2, BoneDataCb::id);

				multiMesh->Add(geom, mat);
			}
		}



	};

}
