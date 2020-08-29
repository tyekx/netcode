#include "NetcodeApp.h"
#include <Netcode/Input/AxisMap.hpp>
#include "DevCameraScript.h"
#include "PlayerBehavior.h"
#include "GunScript.h"
#include "DebugScript.h"
#include "RemoteAvatarScript.h"
#include "Snippets.h"
#include <NetcodeAssetLib/JsonUtility.h>


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

	/*
	GameObject * gunRootObj = gameScene->Create();
	GameObject * gunObj = gameScene->Create();
	LoadComponents(assetManager->Import(L"gun.ncasset"), gunObj);

	Transform * gunRootTransform = gunRootObj->AddComponent<Transform>();
	Transform * gunTransform = gunObj->GetComponent<Transform>();

	gunRootTransform->position = Netcode::Float3{ 0.0f, 130.0f, 0.0f };
	gunTransform->scale = Netcode::Float3{ 18.0f, 18.0f, 18.0f };
	gunRootObj->Parent(avatarController);
	gunObj->Parent(gunRootObj);

	Netcode::Quaternion gunRotation{ -Netcode::C_PIDIV2, -Netcode::C_PIDIV2, 0.0f };

	Script * gunScript = gunObj->AddComponent<Script>();

	auto behav = std::make_unique<GunBehavior>(avatarHitboxes, gunRootObj, Netcode::Float4{ 0.0f, 0.0f, 40.0f, 0.0f }, gunRotation, 28);

	GameObject * debugObj = gameScene->Create();
	Script * debugScript = debugObj->AddComponent<Script>();
	debugScript->SetBehavior(std::make_unique<DebugBehavior>(
	//&behav->localPosition.x, &behav->localPosition.y, &behav->localPosition.z
	&anim->effectors[0].position.x, &anim->effectors[0].position.y, &anim->effectors[0].position.z
	));
	debugScript->Setup(debugObj);


	gunScript->SetBehavior(
	std::move(behav)
	);
	gunScript->Setup(gunObj);

	gameScene->Spawn(gunRootObj);
	gameScene->Spawn(gunObj);
	gameScene->Spawn(debugObj);
	*/

	Animation * anim = avatarHitboxes->AddComponent<Animation>();
	CreateYbotAnimationComponent(avatarModel, anim);
	anim->blackboard->BindController(&movCtrl);
	anim->controller = ybotAnimationSet->CreateController();

	Netcode::PxPtr<physx::PxController> pxController = gameScene->CreateController();
	avatarController->AddComponent<Transform>();
	avatarController->AddComponent<Script>()->SetBehavior(std::make_unique<RemoteAvatarScript>(std::move(pxController)));
	avatarHitboxes->Parent(avatarController);

	gameScene->Spawn(avatarController);
	gameScene->Spawn(avatarHitboxes);
}



void GameApp::CreateLocalAvatar() {
	GameObject * avatarController = gameScene->Create("localAvatarController");
	GameObject * avatarCamera = gameScene->Create("localAvatarCamera");

	avatarCamera->Parent(avatarController);

	Transform * avatarCamTransform = avatarCamera->AddComponent<Transform>();
	avatarCamTransform->position.y = 180.0f;

	Camera * fpsCam = avatarCamera->AddComponent<Camera>();
	fpsCam->ahead = Netcode::Float3{ 0.0f, 0.0f, 1.0f };
	fpsCam->aspect = graphics->GetAspectRatio();
	fpsCam->farPlane = 10000.0f;
	fpsCam->nearPlane = 1.0f;
	fpsCam->up = Netcode::Float3{ 0.0f, 1.0f, 0.0f };

	Netcode::PxPtr<physx::PxController> pxController = gameScene->CreateController();

	Transform * act = avatarController->AddComponent<Transform>();
	act->position = Netcode::Float3{ 0.0f, 0.0f, 200.0f };

	Script * scriptComponent = avatarController->AddComponent<Script>();
	scriptComponent->SetBehavior(std::make_unique<PlayerBehavior>(std::move(pxController), fpsCam));
	scriptComponent->Setup(avatarController);

	gameScene->Spawn(avatarController);
	gameScene->Spawn(avatarCamera);

	gameScene->SetCamera(avatarCamera);
}
