#pragma once

#include <Egg/PhysXScene.h>
#include <Egg/EggMath.h>
#include "GameObject.h"
#include "Scene.h"


/*
dont use any global memory
*/
static physx::PxFilterFlags SimulationFilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags & pairFlags, const void * constantBlock, physx::PxU32 constantBlockSize)
{
	// let triggers through
	if(physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;

		if(filterData0.word0 == 1 || filterData1.word0 == 1) {
			return physx::PxFilterFlag::eSUPPRESS;
		}

		return physx::PxFilterFlag::eDEFAULT;
	}

	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

	// trigger the contact callback for pairs (A,B) where
	// the filtermask of A contains the ID of B and vice versa.
	if((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;

	return physx::PxFilterFlag::eDEFAULT;
}

__declspec(align(16)) class GameScene : public Scene {
protected:
	physx::PxControllerManager * controllerManager;
	physx::PxMaterial * controllerMaterial;

	GameScene() = default;
public:
	PerFrameData perFrameData;
	SsaoData ssaoData;



	GameScene(Egg::Physics::PhysX & px) : GameScene() {
		physx::PxSceneDesc sceneDesc{ px.physics->getTolerancesScale() };
		sceneDesc.gravity = physx::PxVec3{ 0.0f, -981.0f, 0.0f };
		sceneDesc.cpuDispatcher = px.dispatcher;
		sceneDesc.filterShader = SimulationFilterShader;
		controllerMaterial = px.physics->createMaterial(0.5f, 0.6f, 0.6f);

		physx::PxScene * pScene = px.physics->createScene(sceneDesc);
		controllerManager = PxCreateControllerManager(*pScene);
		physx::PxPvdSceneClient * pvdClient = pScene->getScenePvdClient();
		if(pvdClient) {
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}

		Scene::SetPhysXScene(pScene);
	}

	void Setup() {
		ssaoData.Offsets[0] = DirectX::XMFLOAT4A(+1.0f, +1.0f, +1.0f, 0.0f);
		ssaoData.Offsets[1] = DirectX::XMFLOAT4A(-1.0f, -1.0f, -1.0f, 0.0f);

		ssaoData.Offsets[2] = DirectX::XMFLOAT4A(-1.0f, +1.0f, +1.0f, 0.0f);
		ssaoData.Offsets[3] = DirectX::XMFLOAT4A(+1.0f, -1.0f, -1.0f, 0.0f);

		ssaoData.Offsets[4] = DirectX::XMFLOAT4A(+1.0f, +1.0f, -1.0f, 0.0f);
		ssaoData.Offsets[5] = DirectX::XMFLOAT4A(-1.0f, -1.0f, +1.0f, 0.0f);

		ssaoData.Offsets[6] = DirectX::XMFLOAT4A(-1.0f, +1.0f, -1.0f, 0.0f);
		ssaoData.Offsets[7] = DirectX::XMFLOAT4A(+1.0f, -1.0f, +1.0f, 0.0f);

		ssaoData.Offsets[8] = DirectX::XMFLOAT4A(-1.0f, 0.0f, 0.0f, 0.0f);
		ssaoData.Offsets[9] = DirectX::XMFLOAT4A(+1.0f, 0.0f, 0.0f, 0.0f);

		ssaoData.Offsets[10] = DirectX::XMFLOAT4A(0.0f, -1.0f, 0.0f, 0.0f);
		ssaoData.Offsets[11] = DirectX::XMFLOAT4A(0.0f, +1.0f, 0.0f, 0.0f);

		ssaoData.Offsets[12] = DirectX::XMFLOAT4A(0.0f, 0.0f, -1.0f, 0.0f);
		ssaoData.Offsets[13] = DirectX::XMFLOAT4A(0.0f, 0.0f, +1.0f, 0.0f);

		for(int i = 0; i < SsaoData::SAMPLE_COUNT; ++i)
		{
			float s = RandomFloat(0.3f, 1.0f);
			s = s * s;

			DirectX::XMVECTOR v = DirectX::XMVectorScale(DirectX::XMVector4Normalize(DirectX::XMLoadFloat4A(&ssaoData.Offsets[i])), s);

			DirectX::XMStoreFloat4A(&(ssaoData.Offsets[i]), v);
		}

		ssaoData.occlusionRadius = 0.5f;
		ssaoData.occlusionFadeStart = 0.2f;
		ssaoData.occlusionFadeEnd = 1.0f;
		ssaoData.surfaceEpsilon = 0.05f;
	}

	DirectX::XMMATRIX GetView(Transform * transform, Camera * camera) {
		DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&transform->worldPosition);
		DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&camera->up);
		DirectX::XMVECTOR ahead = DirectX::XMLoadFloat3(&camera->ahead);
		return DirectX::XMMatrixLookToRH(eyePos, ahead, up);
	}

	DirectX::XMMATRIX GetProj(Camera * c) {
		return DirectX::XMMatrixPerspectiveFovRH(c->fov, c->aspect, c->nearPlane, c->farPlane);
	}

	void UpdatePerFrameCb() {
		Transform * transform = cameraRef->GetComponent<Transform>();
		Camera * camComponent = cameraRef->GetComponent<Camera>();

		const DirectX::XMMATRIX view = GetView(transform, camComponent);
		const DirectX::XMMATRIX proj = GetProj(camComponent);

		const DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(view, proj);
		DirectX::XMVECTOR vpDet = DirectX::XMMatrixDeterminant(vp);
		const DirectX::XMMATRIX invVp = DirectX::XMMatrixInverse(&vpDet, vp);

		const DirectX::XMMATRIX tex{  0.5f,  0.0f, 0.0f, 0.0f,
									  0.0f, -0.5f, 0.0f, 0.0f,
									  0.0f,  0.0f, 1.0f, 0.0f,
									  0.5f,  0.5f, 0.0f, 1.0f };

		DirectX::XMVECTOR lookToV = DirectX::XMLoadFloat3(&camComponent->ahead);
		DirectX::XMVECTOR upV = DirectX::XMLoadFloat3(&camComponent->up);

		DirectX::XMMATRIX viewFromOrigo = DirectX::XMMatrixLookToRH(DirectX::g_XMZero, lookToV, upV);
		DirectX::XMMATRIX rayDir = DirectX::XMMatrixMultiply(viewFromOrigo, proj);

		DirectX::XMVECTOR rayDirDet = DirectX::XMMatrixDeterminant(rayDir);
		rayDir = DirectX::XMMatrixInverse(&rayDirDet, rayDir);

		DirectX::XMStoreFloat4x4A(&perFrameData.RayDir, DirectX::XMMatrixTranspose(rayDir));

		perFrameData.farZ = camComponent->farPlane;
		perFrameData.nearZ = camComponent->nearPlane;
		perFrameData.fov = camComponent->fov;
		perFrameData.aspectRatio = camComponent->aspect;

		DirectX::XMStoreFloat4x4A(&perFrameData.View, DirectX::XMMatrixTranspose(view));
		DirectX::XMStoreFloat4x4A(&perFrameData.Proj, DirectX::XMMatrixTranspose(proj));

		DirectX::XMStoreFloat4x4A(&perFrameData.ViewProj, DirectX::XMMatrixTranspose(vp));
		DirectX::XMStoreFloat4x4A(&perFrameData.ViewProjInv, DirectX::XMMatrixTranspose(invVp));

		const DirectX::XMFLOAT4 eyePos{ transform->position.x, transform->position.y, transform->position.z, 1.0f };

		DirectX::XMStoreFloat4A(&perFrameData.eyePos, DirectX::XMLoadFloat4(&eyePos));

		DirectX::XMStoreFloat4x4A(&perFrameData.ViewInv, DirectX::XMMatrixTranspose( DirectX::XMMatrixMultiply(proj, invVp) ));
		DirectX::XMStoreFloat4x4A(&perFrameData.ProjInv, DirectX::XMMatrixTranspose( DirectX::XMMatrixMultiply(invVp, view) ));

		//DirectX::XMVECTOR pDet = DirectX::XMMatrixDeterminant(proj);
		//const auto invP = DirectX::XMMatrixInverse(&pDet, proj);
		//DirectX::XMStoreFloat4x4A(&perFrameData.ProjInv, DirectX::XMMatrixTranspose(invP));

		DirectX::XMStoreFloat4x4A(&perFrameData.ProjTex, DirectX::XMMatrixTranspose( DirectX::XMMatrixMultiply( proj, tex ) ));
	}


	physx::PxController * CreateController() {
		physx::PxCapsuleControllerDesc cd;
		cd.behaviorCallback = NULL;
		cd.climbingMode = physx::PxCapsuleClimbingMode::eEASY;
		cd.contactOffset = 0.1f;
		cd.density = 10.0f;
		cd.invisibleWallHeight = 0.0f;
		cd.material = controllerMaterial;
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
		return controllerManager->createController(cd);
	}
};
