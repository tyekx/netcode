#include "NetcodeApp.h"
#include <Netcode/Input/AxisMap.hpp>
#include "Scripts/DebugScript.h"
#include "Scripts/RemotePlayerScript.h"
#include "Scripts/DevCameraScript.h"
#include "Scripts/LocalPlayerScript.h"
#include "Scripts/GunScript.h"
#include "Snippets.h"
#include <NetcodeAssetLib/JsonUtility.h>

#include "Scripts/LocalPlayerWeaponScript.h"


void GameApp::ReloadMap() {
	GameSceneManager * gsm = Service::Get<GameSceneManager>();
	gsm->CloseScene();
	graphics->ClearImportCache();
	
	LoadAssets();
	gsm->ReloadScene();
}

void GameApp::LoadMap(const Netcode::URI::Model & path)
{
	LoadAssets();
	GameSceneManager * gsm = Service::Get<GameSceneManager>();
	gsm->LoadScene(path);
}

void GameApp::Render() {
	graphics->frame->Prepare();

	auto cfgBuilder = graphics->CreateFrameGraphBuilder();
	renderer.CreateComputeFrameGraph(cfgBuilder.get());
	graphics->frame->Run(cfgBuilder->Build(), FrameGraphCullMode::NONE);

	graphics->frame->DeviceSync();

	renderer.ReadbackComputeResults();
	renderer.perFrameData = &gameScene->perFrameData;

	gameScene->Foreach([this](GameObject * gameObject) -> void {
		if(gameObject->IsActive()) {
			transformSystem.Run(gameObject);
			pxSystem.Run(gameObject);
			renderSystem.Run(gameObject);
			lightSystem.Run(gameObject);
		}
	});

	gameScene->UpdatePerFrameCb();

	auto builder = graphics->CreateFrameGraphBuilder();
	renderer.CreateFrameGraph(builder.get());

	graphics->frame->Run(builder->Build(), FrameGraphCullMode::ANY);
	graphics->frame->Present();
	graphics->frame->DeviceSync();
	graphics->frame->CompleteFrame();

	renderer.Reset();
}

void GameApp::Simulate(float dt) {
	totalTime += dt;

	movCtrl.Update();
	pageManager.Update(dt);

	gameScene->GetPhysXScene()->simulate(dt);
	gameScene->GetPhysXScene()->fetchResults(true);

	gameScene->Foreach([this, dt](GameObject * gameObject)->void {
		if(gameObject->IsActive()) {
			scriptSystem.Run(gameObject, dt);
			animSystem.Run(gameObject, dt);
		}
	});
}

void GameApp::LoadServices() {
	Service::Init<AssetManager>(graphics.get());
	Service::Init<Netcode::Physics::PhysX>();
	Service::Init<Netcode::Module::IGraphicsModule *>(graphics.get());

	pxService = Service::Get<Netcode::Physics::PhysX>();
	pxService->CreateResources();

	Service::Init<GameSceneManager>();


	gameScene = Service::Get<GameSceneManager>()->GetScene();

	gameScene->Setup();
}

void GameApp::CreateUI() {
	Ref<LoginPage> loginPage = pageManager.CreatePage<LoginPage>(*pxService->physics);
	Ref<ServerBrowserPage> serverBrowserPage = pageManager.CreatePage<ServerBrowserPage>(*pxService->physics);
	Ref<LoadingPage> loadingPage = pageManager.CreatePage<LoadingPage>(*pxService->physics);

	loginPage->InitializeComponents();
	serverBrowserPage->InitializeComponents();
	loadingPage->InitializeComponents();

	pageManager.AddPage(loginPage);
	pageManager.AddPage(serverBrowserPage);
	pageManager.AddPage(loadingPage);

	renderer.ui_Input = &pageManager;
}

void GameApp::CreateAxisMapping() {
	Ref<Netcode::AxisMapBase> axisMap = std::make_shared<Netcode::AxisMap<AxisEnum>>(std::initializer_list<Netcode::AxisData<AxisEnum>> {
		Netcode::AxisData<AxisEnum> { AxisEnum::VERTICAL, Netcode::KeyCode::W, Netcode::KeyCode::S },
		Netcode::AxisData<AxisEnum> { AxisEnum::HORIZONTAL, Netcode::KeyCode::A, Netcode::KeyCode::D },
		Netcode::AxisData<AxisEnum> { AxisEnum::FIRE1, Netcode::KeyCode::MOUSE_LEFT, Netcode::KeyCode::UNDEFINED },
		Netcode::AxisData<AxisEnum> { AxisEnum::FIRE2, Netcode::KeyCode::MOUSE_RIGHT, Netcode::KeyCode::UNDEFINED },
		Netcode::AxisData<AxisEnum> { AxisEnum::JUMP, Netcode::KeyCode::SPACE, Netcode::KeyCode::UNDEFINED },
		Netcode::AxisData<AxisEnum> { AxisEnum::DEV_CAM_X, Netcode::KeyCode::RIGHT, Netcode::KeyCode::LEFT },
		Netcode::AxisData<AxisEnum> { AxisEnum::DEV_CAM_Z, Netcode::KeyCode::HOME, Netcode::KeyCode::PAGE_UP },
		Netcode::AxisData<AxisEnum> { AxisEnum::DEV_CAM_Y, Netcode::KeyCode::DOWN, Netcode::KeyCode::UP },
	});

	Netcode::Input::SetAxisMap(std::move(axisMap));

	Netcode::Input::OnKeyPressed->Subscribe([this](Netcode::Key key, Netcode::KeyModifier modifiers) -> void {
		if(modifiers == Netcode::KeyModifier::NONE) {
			if(key == Netcode::KeyCode::F5) {
				ReloadMap();
				Log::Debug("Map reloaded");
			}
		}

		if(modifiers == Netcode::KeyModifier::CTRL && key == Netcode::KeyCode::C) {
			GameSceneManager * gsm = Service::Get<GameSceneManager>();
			GameScene * scene = gsm->GetScene();
			GameObject * obj = scene->FindByName("localAvatarCamera");
			if(obj != nullptr) {
				scene->SetCamera(obj);
				Log::Debug("Entering avatar");
			} else {
				Log::Debug("localAvatarCamera was not found");
			}
		}
	});
}


void GameApp::LoadSystems() {
	renderer.CreatePermanentResources(graphics.get());
	animSystem.SetMovementController(&movCtrl);
	animSystem.renderer = &renderer;
	renderSystem.renderer = &renderer;
	lightSystem.renderer = &renderer;
	renderer.sceneLights = &lightSystem.lights;
}

void GameApp::LoadAssets() {
	AssetManager * assetManager = Service::Get<AssetManager>();

	CreateLocalAvatar();
	//CreateRemoteAvatar();
	
	{
		Ref<Netcode::TextureBuilder> textureBuilder = graphics->CreateTextureBuilder();
		textureBuilder->LoadTextureCube(L"compiled/textures/envmaps/cloudynoon.dds");
		Ref<Netcode::GpuResource> cloudynoonTexture = textureBuilder->Build();
		graphics->resources->SetDebugName(cloudynoonTexture, L"Cloudynoon TextureCube");

		Ref<Netcode::FrameGraphBuilder> frameGraphBuilder = graphics->CreateFrameGraphBuilder();

		Ref<Netcode::GpuResource> prefilteredEnvMap = renderer.PrefilterEnvMap(frameGraphBuilder.get(), cloudynoonTexture);
		Ref<Netcode::GpuResource> preIntegratedBrdf = renderer.PreIntegrateBrdf(frameGraphBuilder.get());
		graphics->frame->Run(frameGraphBuilder->Build(), Netcode::Graphics::FrameGraphCullMode::NONE);
		graphics->frame->DeviceSync();

		renderer.SetGlobalEnvMap(prefilteredEnvMap, preIntegratedBrdf);
	}

	{
		auto planeActor = physx::PxCreatePlane(*pxService->physics, physx::PxPlane{ 0.0f, 1.0f, 0.0f, 0.0f }, *pxService->defaultMaterial);
		gameScene->SpawnPhysxActor(planeActor);
	}
}

void GameApp::CreateRemoteAvatar() {
	AssetManager * assetManager = Service::Get<AssetManager>();

	GameObject * avatarController = gameScene->Create("remoteAvatarController");
	GameObject * avatarHitboxes = gameScene->Create("remoteAvatarHitboxes");

	Netcode::Asset::Model * avatarModel = assetManager->Import(L"compiled/models/ybot.ncasset");

	if(ybotAnimationSet == nullptr) {
		ybotAnimationSet = std::make_shared<AnimationSet>(graphics.get(), avatarModel->animations, avatarModel->bones);
	}

	Animation * anim = avatarHitboxes->AddComponent<Animation>();
	CreateYbotAnimationComponent(avatarModel, anim);
	anim->blackboard->BindController(&movCtrl);
	anim->controller = ybotAnimationSet->CreateController();

	Netcode::PxPtr<physx::PxController> pxController = gameScene->CreateController();
	avatarController->AddComponent<Transform>();
	avatarController->AddComponent<Script>()->AddScript(std::make_unique<RemotePlayerScript>(std::move(pxController)));
	avatarHitboxes->Parent(avatarController);

	gameScene->Spawn(avatarController);
	gameScene->Spawn(avatarHitboxes);
}

void GameApp::CreateLocalAvatar() {
	GameObject * avatarRoot = gameScene->Create("localAvatarRoot");
	GameObject * avatarCamera = gameScene->Create("localAvatarCamera");
	GameObject * avatarAttachmentNode = gameScene->Create("localAvatarAttachmentNode");
	GameObject * avatarWeaponOffset = gameScene->Create("localAvatarWeaponOffset");
	GameObject * avatarWeapon = gameScene->Create("localAvatarWeapon");

	avatarRoot->AddChild(avatarCamera);
	avatarCamera->AddChild(avatarAttachmentNode);
	avatarAttachmentNode->AddChild(avatarWeaponOffset);
	avatarWeaponOffset->AddChild(avatarWeapon);

	avatarWeaponOffset->AddComponent<Transform>()->position = Netcode::Float3{ -12.0f, -13.0f, 33.0f };
	
	{
		Transform * wTr = avatarWeapon->AddComponent<Transform>();
		wTr->rotation = Netcode::Quaternion{ 0.0f, -Netcode::C_PIDIV2, 0.0f };
		
		GameObject * debugObject = gameScene->Create("debugHelper");
		debugObject->AddComponent<Script>()->AddScript(std::make_unique<DebugScript>(&wTr->position.x, &wTr->position.y, &wTr->position.z));
		gameScene->Spawn(debugObject);
		
		AssetManager * assetManager = Service::Get<AssetManager>();
		ClientAssetConverter cac{ nullptr, nullptr, nullptr };
		cac.ConvertComponents(avatarWeapon, assetManager->Import(L"compiled/models/gun_2.ncasset"));

		avatarWeapon->AddComponent<Script>()->AddScript(std::make_unique<LocalPlayerWeaponScript>(wTr, &movCtrl));
	}

	Transform * avatarCamTransform = avatarCamera->AddComponent<Transform>();
	avatarCamTransform->position.y = 90.0f;

	Camera * fpsCam = avatarCamera->AddComponent<Camera>();
	fpsCam->ahead = Netcode::Float3{ 0.0f, 0.0f, 1.0f };
	fpsCam->aspect = graphics->GetAspectRatio();
	fpsCam->farPlane = 10000.0f;
	fpsCam->nearPlane = 1.0f;
	fpsCam->up = Netcode::Float3{ 0.0f, 1.0f, 0.0f };

	avatarAttachmentNode->AddComponent<Transform>();

	Netcode::PxPtr<physx::PxController> pxController = gameScene->CreateController();

	Transform * act = avatarRoot->AddComponent<Transform>();
	act->position = Netcode::Float3{ 0.0f, 0.0f, 200.0f };

	Script * scriptComponent = avatarRoot->AddComponent<Script>();
	scriptComponent->AddScript(std::make_unique<LocalPlayerScript>(std::move(pxController), avatarCamera, avatarAttachmentNode));

	gameScene->Spawn(avatarRoot);
	gameScene->Spawn(avatarCamera);
	gameScene->Spawn(avatarAttachmentNode);
	gameScene->Spawn(avatarWeaponOffset);
	gameScene->Spawn(avatarWeapon);
}



