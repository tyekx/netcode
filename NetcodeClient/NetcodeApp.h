#pragma once

#include <DirectXColors.h>

#include <Netcode/Input.h>
#include <Netcode/BasicGeometry.h>
#include <Netcode/DebugPhysx.h>
#include <Netcode/MathExt.h>
#include <Netcode/Modules.h>
#include <Netcode/Service.hpp>
#include <Netcode/UI/PageManager.h>

#include "Asset/ClientConverters.h"

#include "Asset.h"
#include "GameObject.h"
#include "Systems.h"
#include "Scene.h"
#include "PhysxHelpers.h"
#include "Services.h"
#include "UITest.h"

#include <Netcode/URI/Model.h>
#include <Netcode/System/Dispatcher.hpp>
#include <Netcode/System/FpsCounter.h>
#include <Netcode/System/SystemClock.h>
#include <Netcode/System/System.h>

#include <Netcode/Network/ClientSession.h>

#include "Network/GameServer.h"
#include "Network/GameClient.h"

using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceState;
using Netcode::Graphics::FrameGraphCullMode;

namespace nn = Netcode::Network;
namespace np = Netcode::Protocol;

class GameApp : public Netcode::Module::AApp, Netcode::Module::TAppEventHandler {
public:
	Netcode::Dispatcher mainThreadDispatcher;
	GraphicsEngine renderer;
	Netcode::MovementController movCtrl;
	TransformSystem transformSystem;
	ScriptSystem scriptSystem;
	ScriptFixedSystem scriptFixedSystem;
	RenderSystem renderSystem;
	AnimationSystem animSystem;
	LightSystem lightSystem;
	PhysXSystem pxSystem;
	Netcode::UI::PageManager pageManager;
	Netcode::Physics::PhysX * pxService;
	GameScene * gameScene;
	Ref<AnimationSet> ybotAnimationSet;
	GameServer gameServer;
	GameClient gameClient;
	Netcode::URI::Model mapAsset;
	Netcode::FrameCounter fpsCounter;
	Netcode::GameClock gameClock;
	std::wstring fpsValue;
	UserData user;
	HostMode hostMode;

	void LoadSystems();

	void ReloadMap();

	void LoadMap(const Netcode::URI::Model & path);

	void Render();

	void Simulate();

	void LoadServices();

	void CreateUI();

	void CreateAxisMapping();

	void LoadAssets();

	void CreateLocalAvatar();

	virtual void OnResized(int w, int h) override {
		float asp = graphics->GetAspectRatio();
		if(GameObject * cameraObj = gameScene->GetCamera(); cameraObj != nullptr) {
			Camera * cam = cameraObj->GetComponent<Camera>();
			if(cam != nullptr) {
				cam->aspect = asp;
			}
		}
		pageManager.WindowResized(Netcode::UInt2{ static_cast<uint32_t>(w), static_cast<uint32_t>(h) });
		renderer.OnResize(w, h);
	}

	virtual void AddAppEventHandlers(Netcode::Module::AppEventSystem * eventSystem) override {
		AApp::AddAppEventHandlers(eventSystem);

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

		gameClock.SetEpoch(Netcode::SystemClock::LocalNow() - Netcode::Timestamp{});

		LoadServices();
		LoadSystems();
		CreateAxisMapping();
		LoadMap(L"mat_test_map.json");
		CreateUI();
	}

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override {
		while(window->KeepRunning()) {
			auto st = Netcode::SystemClock::LocalNow();
			
			gameClock.Tick();
			
			window->ProcessMessages();
			events->Dispatch();

			Netcode::Input::UpdateAxisMap();

			const bool isNetworkTick = gameClient.IncludeNetworkTick();
			
			if(isNetworkTick) {
				if(gameClient.IsConnected())
					gameClient.Receive();
			}
			
			Simulate();

			if(isNetworkTick) {
				if(gameClient.IsConnected())
					gameClient.SendDebug();
			}

			if(isNetworkTick && hostMode == HostMode::LISTEN) {
				gameServer.Tick();
			}
			
			Render();
			
			mainThreadDispatcher.Run();

			window->CompleteFrame();

			Sleep(2);
			
			fpsCounter.Update(Netcode::SystemClock::LocalNow() - st);
		}
	}

	/*
	Properly shutdown the application
	*/
	virtual void Exit() override {
		renderer.ui_Input = nullptr;
		pageManager.Destruct();
		Service::Clear();
		ShutdownModule(network.get());
		ShutdownModule(audio.get());
		ShutdownModule(graphics.get());
		ShutdownModule(window.get());
	}
};

