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

		transform->position = Netcode::Float3{ 0.0f, 0.0f, 0.0f };

		camera->ahead = Netcode::Float3{ 0.0f, 0.0f, 1.0f };
		camera->aspect = 1.0f;
		camera->farPlane = 1.0f;
		camera->nearPlane = 0.0f;
		camera->fov = Netcode::C_PI / 3.0f;
	}

	void SetScreenSize(const Netcode::UInt2 & dim) {
		screenSize = dim;
		cameraRef->GetComponent<Camera>()->aspect = static_cast<float>(dim.x) / static_cast<float>(dim.y);
		cameraRef->GetComponent<Transform>()->position = Netcode::Float3{ static_cast<float>(dim.x) / 2.0f ,static_cast<float>(dim.y) / 2.0f, 0.0f };
	}

	UIButtonPrefab CreateButton(std::wstring text, const Netcode::Float2 & size, const Netcode::Float2 & pos, float zIndex, Netcode::SpriteFontRef font, Netcode::ResourceViewsRef bgTex, const Netcode::UInt2 & texSize) {
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

	UILabel CreateLabel(std::wstring text, Netcode::SpriteFontRef font, const Netcode::Float2 & pos) {
		UILabel label = CreateLabel();

		label.SetPosition(pos);
		label.SetFont(font);
		label.SetText(std::move(text));

		return label;
	}

	UITextBox CreateTextBox(const Netcode::Float2 & size, const Netcode::Float2 & pos, Netcode::SpriteFontRef font, Netcode::ResourceViewsRef bgTex, const Netcode::UInt2 & texSize) {
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

		const Netcode::Vector3 eyePos = transform->worldPosition;

		const Netcode::Matrix view = Netcode::LookToMatrix(eyePos, camComponent->ahead, camComponent->up);
		const Netcode::Matrix proj = Netcode::OrtographicMatrix(static_cast<float>(screenSize.x), static_cast<float>(screenSize.y), camComponent->nearPlane, camComponent->farPlane);

		const Netcode::Matrix vp = view * proj;
		const Netcode::Matrix invVp = vp.Invert();

		const Netcode::Matrix tex = Netcode::Float4x4{ 0.5f,  0.0f, 0.0f, 0.0f,
													  0.0f, -0.5f, 0.0f, 0.0f,
													  0.0f,  0.0f, 1.0f, 0.0f,
													  0.5f,  0.5f, 0.0f, 1.0f };

		Netcode::Matrix rayDir = Netcode::LookToMatrix(Netcode::Float3{ }, camComponent->ahead, camComponent->up) * proj;
		rayDir = rayDir.Invert();

		perFrameData.farZ = camComponent->farPlane;
		perFrameData.nearZ = camComponent->nearPlane;
		perFrameData.fov = camComponent->fov;
		perFrameData.aspectRatio = camComponent->aspect;

		perFrameData.RayDir = rayDir.Transpose();
		perFrameData.View = view.Transpose();
		perFrameData.Proj = proj.Transpose();
		perFrameData.ViewProj = vp.Transpose();
		perFrameData.ViewProjInv = invVp.Transpose();

		perFrameData.eyePos = eyePos.XYZ1();

		Netcode::Matrix projInv = invVp * view;
		Netcode::Matrix viewInv = proj * invVp;

		perFrameData.ViewInv = viewInv.Transpose();
		perFrameData.ProjInv = projInv.Transpose();
		perFrameData.ProjTex = (proj * tex).Transpose();
	}


	void Update() {
		UpdatePerFrameCb();

		
	}

	void Spawn(UIObject * object) {
		// actual spawning handled by UISystem
		object->Spawn();
	}
};
