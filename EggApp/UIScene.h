#pragma once

#include "Scene.h"
#include "UIObject.h"
#include <Egg/EggMath.h>
#include "PhysxHelpers.h"

class UIScene : public Scene<UIObject> {
public:

	PerFrameData perFrameData;
	DirectX::XMUINT2 screenSize;
	bool lmbHeld;

	DirectX::XMMATRIX GetView(Transform * transform, Camera * camera) {
		DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&transform->position);
		DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&camera->up);
		DirectX::XMVECTOR ahead = DirectX::XMLoadFloat3(&camera->ahead);
		return DirectX::XMMatrixLookToRH(eyePos, ahead, up);
	}

	DirectX::XMMATRIX GetProj(Camera * c) {
		return DirectX::XMMatrixOrthographicRH(static_cast<float>(screenSize.x), static_cast<float>(screenSize.y), c->nearPlane, c->farPlane);
	}

	UIScene() = default;

public:
	~UIScene() noexcept = default;

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

		transform->position = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };

		camera->ahead = DirectX::XMFLOAT3{ 0.0f, 0.0f, 1.0f };
		camera->aspect = 1.0f;
		camera->farPlane = 1.0f;
		camera->nearPlane = 0.0f;
		camera->fov = DirectX::XM_PI / 3.0f;
	}

	void SetScreenSize(const DirectX::XMUINT2 & dim) {
		screenSize = dim;
		cameraRef->GetComponent<Camera>()->aspect = static_cast<float>(dim.x) / static_cast<float>(dim.y);
		cameraRef->GetComponent<Transform>()->position = DirectX::XMFLOAT3{ static_cast<float>(dim.x) / 2.0f ,static_cast<float>(dim.y) / 2.0f, 0.0f };
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

		
	}

	void Spawn(UIObject * object) {
		// actual spawning handled by UISystem
		object->Spawn();
	}
};
