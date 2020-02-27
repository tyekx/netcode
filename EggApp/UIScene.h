#pragma once

#include "Scene.h"
#include "UIObject.h"
#include <Egg/EggMath.h>
#include "PhysxHelpers.h"

class UIScene : public Scene<UIObject> {
	DirectX::XMINT2 screenSize;
	PerFrameData perFrameData;

	DirectX::XMMATRIX GetView(Transform * transform, Camera * camera) {
		DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&transform->worldPosition);
		DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&camera->up);
		DirectX::XMVECTOR ahead = DirectX::XMLoadFloat3(&camera->ahead);
		return DirectX::XMMatrixLookToRH(eyePos, ahead, up);
	}

	DirectX::XMMATRIX GetProj(Camera * c) {
		return DirectX::XMMatrixPerspectiveFovRH(c->fov, c->aspect, c->nearPlane, c->farPlane);
	}

	UIScene() = default;

public:
	UIScene(Egg::Physics::PhysX & px) : UIScene() {
		physx::PxSceneDesc sceneDesc{ px.physics->getTolerancesScale() };
		sceneDesc.gravity = physx::PxVec3{ 0.0f, -981.0f, 0.0f };
		sceneDesc.cpuDispatcher = px.dispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

		physx::PxScene * pScene = px.physics->createScene(sceneDesc);
		physx::PxPvdSceneClient * pvdClient = pScene->getScenePvdClient();
		if(pvdClient) {
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}

		Scene::SetPhysXScene(pScene);

		cameraRef = Create();
		Transform* transform = cameraRef->AddComponent<Transform>();
		Camera *camera = cameraRef->AddComponent<Camera>();

		transform->position = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1000.0f };

		camera->ahead = DirectX::XMFLOAT3{ 0.0f, 0.0f, 1.0f };
		camera->aspect = 1.0f;
		camera->farPlane = 1000.0f;
		camera->nearPlane = 1.0f;
		camera->fov = DirectX::XM_PI / 3.0f;
	}

	void SetScreenSize(const DirectX::XMUINT2 & dim) {
		screenSize = DirectX::XMINT2{ static_cast<int32_t>(dim.x), static_cast<int32_t>(dim.y) };
		cameraRef->GetComponent<Camera>()->aspect = static_cast<float>(dim.x) / static_cast<float>(dim.y);
	}

	void UpdatePerFrameCb() {
		Transform * transform = cameraRef->GetComponent<Transform>();
		Camera * camComponent = cameraRef->GetComponent<Camera>();

		const DirectX::XMMATRIX view = GetView(transform, camComponent);
		const DirectX::XMMATRIX proj = GetProj(camComponent);

		const DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(view, proj);
		DirectX::XMVECTOR vpDet = DirectX::XMMatrixDeterminant(vp);
		const DirectX::XMMATRIX invVp = DirectX::XMMatrixInverse(&vpDet, vp);

		const DirectX::XMMATRIX tex{ 0.5f,  0.0f, 0.0f, 0.0f,
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

		DirectX::XMStoreFloat4x4A(&perFrameData.ViewInv, DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(proj, invVp)));
		DirectX::XMStoreFloat4x4A(&perFrameData.ProjInv, DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(invVp, view)));

		DirectX::XMStoreFloat4x4A(&perFrameData.ProjTex, DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(proj, tex)));
	}


	void Update() {
		UpdatePerFrameCb();

		float lmb = Egg::Input::GetAxis("Fire");
		DirectX::XMINT2 mousePos = Egg::Input::GetMousePos();

		if(lmb == 0.0f) {
			return;
		}

		DirectX::XMFLOAT4 ndcMousePos{
			2.0f * (static_cast<float>(mousePos.x) / static_cast<float>(screenSize.x)) - 1.0f,
			2.0f * (1.0f - (static_cast<float>(mousePos.y) / static_cast<float>(screenSize.y))) -1.0f,
			0.0f,
			1.0f
		};

		DirectX::XMVECTOR ndcMousePosV = DirectX::XMLoadFloat4(&ndcMousePos);
		DirectX::XMMATRIX viewProjInvV = DirectX::XMLoadFloat4x4A(&perFrameData.ViewProjInv);
		DirectX::XMMATRIX rayDirV = DirectX::XMLoadFloat4x4A(&perFrameData.RayDir);

		DirectX::XMVECTOR modelSpaceMousePos = DirectX::XMVector4Transform(ndcMousePosV, DirectX::XMMatrixTranspose(viewProjInvV));
		DirectX::XMVECTOR rayDirVector = DirectX::XMVector4Transform(ndcMousePosV, DirectX::XMMatrixTranspose(rayDirV));

		rayDirVector = DirectX::XMVector3Normalize(rayDirVector);

		modelSpaceMousePos = DirectX::XMVectorDivide(modelSpaceMousePos, DirectX::XMVectorSwizzle<3, 3, 3, 3>(modelSpaceMousePos));

		DirectX::XMFLOAT3 raycastRayDir;
		DirectX::XMFLOAT3 raycastRayStart;

		DirectX::XMStoreFloat3(&raycastRayStart, modelSpaceMousePos);
		DirectX::XMStoreFloat3(&raycastRayDir, rayDirVector);

		physx::PxVec3 pxRayStart = ToPxVec3(raycastRayStart);
		physx::PxVec3 pxRayDir = ToPxVec3(raycastRayDir);

		physx::PxQueryFilterData filterData;
		filterData.data.word0 = PHYSX_COLLIDER_TYPE_UI;

		physx::PxRaycastBuffer outRaycastResult;

		if(pxScene->raycast(pxRayStart, pxRayDir, 1000.0f, outRaycastResult)) {
			uint32_t numHits = outRaycastResult.getNbAnyHits();

			for(uint32_t i = 0; i < numHits; ++i) {

			}
		}
		
	}
};
