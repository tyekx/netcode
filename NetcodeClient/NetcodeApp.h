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
#include <Netcode/Network/ServerSession.h>
#include <Netcode/Network/Service.h>

using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceState;
using Netcode::Graphics::FrameGraphCullMode;

namespace nn = Netcode::Network;
namespace np = Netcode::Protocol;

struct Connection : public nn::ConnectionBase {
	RedundancyBuffer redundancyBuffer;
	GameObject* gameObject;
	uint32_t localActionIndex;
	uint32_t remoteActionIndex;

	Connection(boost::asio::io_context & ioc) : nn::ConnectionBase{ ioc },
		redundancyBuffer{}, gameObject{ nullptr }, localActionIndex{ 0 }, remoteActionIndex{ 0 } { }
};

class GameApp : public Netcode::Module::AApp, Netcode::Module::TAppEventHandler {
public:
	Netcode::Dispatcher mainThreadDispatcher;
	GraphicsEngine renderer;
	Netcode::Stopwatch stopwatch;
	Netcode::MovementController movCtrl;
	TransformSystem transformSystem;
	ScriptSystem scriptSystem;
	RenderSystem renderSystem;
	AnimationSystem animSystem;
	LightSystem lightSystem;
	PhysXSystem pxSystem;
	Netcode::UI::PageManager pageManager;
	Netcode::Physics::PhysX * pxService;
	GameScene * gameScene;
	Ref<AnimationSet> ybotAnimationSet;
	Ref<Connection> playerConnection;
	Ref<nn::ClientSession> clientSession;
	Ref<nn::ServerSession> serverSession;
	Netcode::URI::Model mapAsset;
	Netcode::FrameCounter fpsCounter;
	std::wstring fpsValue;
	UserData user;
	uint32_t processedTick;

	float totalTime;

	void LoadSystems();

	void ReloadMap();

	void LoadMap(const Netcode::URI::Model & path);

	void Render();

	void Simulate(float dt);

	void LoadServices();

	void CreateUI();

	void CreateAxisMapping();

	void LoadAssets();

	void CreateRemoteAvatar();

	void CreateLocalAvatar();

	bool IncludeNetworkTick() {
		if(playerConnection == nullptr) {
			return false;
		}

		const uint32_t tc = playerConnection->tickCounter.load(std::memory_order_acquire);

		if(processedTick >= tc) {
			return false;
		}

		processedTick = tc;
		return true;
	}

	ReconciliationBuffer * reconciliationBuffer;
	RedundancyBuffer * redundancyBuffer;

	void ClientReceiveNetworkUpdate() {
		std::vector<nn::GameMessage> updates;
		playerConnection->sharedQueue.GetIncomingPackets(updates);

		if(updates.empty()) {
			return;
		}

		for(const nn::GameMessage& gm : updates) {
			/*
			if(gm.update->Content_case() == np::Update::ContentCase::kServerUpdate) {
				const np::ServerUpdate& update = gm.update->server_update();
				// use this to check for duplication or older packages
				const uint32_t mid = update.id();
				redundancyBuffer->Confirm(mid);
				
				// use this to erase from redundancy buffer
				const uint32_t remoteRecvId = update.received_id();

				for(const np::ActionResult& actionResult : update.action_results()) {
					reconciliationBuffer->Reconcile(actionResult);
				}
			}
			*/
		}
	}

	void ClientSendNetworkUpdate() {
		
	}

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

		LoadServices();
		LoadSystems();
		CreateAxisMapping();
		CreateUI();
		//LoadMap(L"mat_test_map.json");
	}

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override {
		/*double targetFrametime = 1.0 / 144.0;

		Netcode::Duration d = std::chrono::duration_cast<Netcode::Duration>(std::chrono::duration<double>(targetFrametime));*/
		
		while(window->KeepRunning()) {
			Netcode::Timestamp st = Netcode::SystemClock::LocalNow();
			
			window->ProcessMessages();
			events->Dispatch();

			float dt = stopwatch.Restart();
			Netcode::Input::UpdateAxisMap(dt);

			const bool isNetworkTick = IncludeNetworkTick();

			serverSession->GetService()->RunFilters();
			
			if(isNetworkTick) {
				ClientReceiveNetworkUpdate();
			}
			
			Simulate(dt);

			if(isNetworkTick) {
				ClientSendNetworkUpdate();
			}

			Render();
			
			mainThreadDispatcher.Run();

			window->CompleteFrame();

			Netcode::SleepFor(std::chrono::milliseconds(2));
			
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

