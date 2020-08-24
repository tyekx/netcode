#pragma once

#include <DirectXColors.h>

#include <Netcode/Input.h>
#include <Netcode/BasicGeometry.h>
#include <Netcode/DebugPhysx.h>
#include <Netcode/MathExt.h>
#include <Netcode/Modules.h>
#include <Netcode/Stopwatch.h>
#include <Netcode/Service.hpp>
#include <Netcode/UI/PageManager.h>

#include "Asset.h"
#include "GameObject.h"
#include "Systems.h"
#include "Scene.h"
#include "PhysxHelpers.h"
#include "Services.h"
#include "UITest.h"

#include <Netcode/URI/Model.h>
#include <Netcode/Graphics/Material.h>
#include <json11.hpp>

using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceState;
using Netcode::Graphics::FrameGraphCullMode;

class GameApp : public Netcode::Module::AApp, Netcode::Module::TAppEventHandler {
	Netcode::Stopwatch stopwatch;
	Netcode::Physics::PhysX px;
	Netcode::PxPtr<physx::PxMaterial> defaultPhysxMaterial;
	Netcode::MovementController movCtrl;
	TransformSystem transformSystem;
	ScriptSystem scriptSystem;
	RenderSystem renderSystem;
	AnimationSystem animSystem;
	PhysXSystem pxSystem;
	Netcode::UI::PageManager pageManager;
	GameScene * gameScene;
	Ref<AnimationSet> ybotAnimationSet;
	Ref<Netcode::Network::GameSession> gameSession;
	Netcode::URI::Model mapAsset;

	float totalTime;

	void LoadGameObjectFromJson(const json11::Json::object & values);

	void LoadSystems();

	void ReloadMap();

	void LoadMap(const Netcode::URI::Model & path);

	void Render();

	void Simulate(float dt);

	void LoadServices();

	void ConnectServer() {
		gameSession = network->CreateClient();
	}

	void CreateUI();

	void CreateAxisMapping();

	void LoadAssets();

	void CreateRemoteAvatar();

	void CreateLocalAvatar();

	void LoadComponents(Netcode::Asset::Model * model, GameObject * gameObject);

	void LoadColliderComponent(Netcode::Asset::Model * model, Collider * colliderComponent);

	void LoadModelComponent(Netcode::Asset::Model * model, Model * modelComponent);

public:

	virtual void OnResized(int w, int h) override {
		float asp = graphics->GetAspectRatio();
		if(gameScene->GetCamera() != nullptr) {
			gameScene->GetCamera()->GetComponent<Camera>()->aspect = asp;
		}
		pageManager.WindowResized(Netcode::UInt2{ static_cast<uint32_t>(w), static_cast<uint32_t>(h) });
		renderSystem.renderer.OnResize(w, h);
	}

	virtual void AddAppEventHandlers(Netcode::Module::AppEventSystem * eventSystem) override {
		Netcode::Module::AApp::AddAppEventHandlers(eventSystem);

		eventSystem->AddHandler(this);
	}

	/*
	Initialize modules
	*/
	virtual void Setup(Netcode::Module::IModuleFactory * factory) override {
		events = std::make_unique<Netcode::Module::AppEventSystem>();

		window = factory->CreateWindowModule(this, 0);
		graphics = factory->CreateGraphicsModule(this, 0);
		audio = factory->CreateAudioModule(this, 0);
		network = factory->CreateNetworkModule(this, 0);

		StartModule(window.get());
		StartModule(graphics.get());
		StartModule(audio.get());
		StartModule(network.get());

		if(window) {
			window->ShowWindow();
		}

		AddAppEventHandlers(events.get());

		stopwatch.Start();

		px.CreateResources();

		LoadServices();
		LoadSystems();
		CreateAxisMapping();
		CreateUI();
		LoadMap(L"test_map.json");
	}

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override {
		while(window->KeepRunning()) {
			window->ProcessMessages();
			
			events->Dispatch();

			float dt = stopwatch.Restart();
			Netcode::Input::UpdateAxisMap(dt);

			Simulate(dt);

			Render();

			window->CompleteFrame();
		}
	}

	/*
	Properly shutdown the application
	*/
	virtual void Exit() override {
		defaultPhysxMaterial.Reset();
		renderSystem.renderer.ui_Input = nullptr;
		pageManager.Destruct();
		Service::Clear();
		px.ReleaseResources();
		ShutdownModule(network.get());
		ShutdownModule(audio.get());
		ShutdownModule(graphics.get());
		ShutdownModule(window.get());
	}
};

