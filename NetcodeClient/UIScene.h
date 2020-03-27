#pragma once

#include "Scene.h"
#include "UIObject.h"
#include <Netcode/NetcodeMath.h>
#include "PhysxHelpers.h"
#include "UIButtonPrefab.h"
#include "UIPagePrefab.h"
#include "UITextBox.h"
#include "UILabel.h"
#include "UISpinner.h"
#include "UIServerRow.h"

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

	UIScene(Netcode::Physics::PhysX & px) : UIScene() {
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

	UIButtonPrefab CreateButton(std::wstring text, const DirectX::XMFLOAT2 & size, const DirectX::XMFLOAT2 & pos, float zIndex, Netcode::SpriteFontRef font, Netcode::ResourceViewsRef bgTex, const DirectX::XMUINT2 & texSize) {
		UIObject * background = Create();
		UIObject * textObject = Create();

		UIButtonPrefab btn(background, textObject);

		btn.SetSize(size);
		btn.SetPosition(pos);
		btn.SetZIndex(zIndex);
		btn.SetFont(std::move(font));
		btn.SetText(std::move(text));
		btn.SetBackgroundImage(std::move(bgTex), texSize);

		Spawn(background);
		Spawn(textObject);

		return btn;
	}

	UISpinner CreateSpinner() {
		UIObject * root = Create();

		Spawn(root);

		return UISpinner(root);
	}

	UIPagePrefab CreatePage() {
		UIObject * root = Create();

		Spawn(root);

		return UIPagePrefab(root);
	}

	UITextBox CreateTextBox() {
		UIObject * root = Create();
		UIObject * txt = Create();

		Spawn(root);
		Spawn(txt);

		return UITextBox(root, txt);
	}

	UILabel CreateLabel() {
		UIObject * root = Create();
		UIObject * txt = Create();

		Spawn(root);
		Spawn(txt);

		return UILabel(root, txt);
	}

	UILabel CreateLabel(std::wstring text, Netcode::SpriteFontRef font, const DirectX::XMFLOAT2 & pos) {
		UILabel label = CreateLabel();

		label.SetPosition(pos);
		label.SetFont(font);
		label.SetText(std::move(text));

		return label;
	}

	UITextBox CreateTextBox(const DirectX::XMFLOAT2 & size, const DirectX::XMFLOAT2 & pos, Netcode::SpriteFontRef font, Netcode::ResourceViewsRef bgTex, const DirectX::XMUINT2 & texSize) {
		UITextBox tb = CreateTextBox();
		tb.SetPosition(pos);
		tb.SetBackgroundImage(bgTex, texSize);
		tb.SetFont(font);
		tb.SetPosition(pos);
		tb.SetSize(size);
		return tb;
	}

	UIServerRow CreateServerRow() {
		UIObject * o1 = Create();
		UIObject * o2 = Create();
		UIObject * o3 = Create();
		UIObject * o4 = Create();

		Spawn(o1);
		Spawn(o2);
		Spawn(o3);
		Spawn(o4);

		o2->Parent(o1);
		o3->Parent(o1);
		o4->Parent(o1);

		return UIServerRow(o1, o2, o3, o4);
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
