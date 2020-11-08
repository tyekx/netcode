#include "NetcodeApp.h"
#include <Netcode/Input/AxisMap.hpp>
#include "Scripts/DebugScript.h"
#include "Scripts/RemotePlayerScript.h"
#include "Scripts/DevCameraScript.h"
#include "Scripts/LocalPlayerScript.h"
#include "Scripts/GunScript.h"
#include "Snippets.h"
#include <NetcodeAssetLib/JsonUtility.h>
#include <Netcode/UI/Button.h>
#include <Netcode/UI/TextBox.h>
#include <Netcode/Network/CompletionToken.h>
#include <Netcode/Network/Response.hpp>
#include <Netcode/Network/Cookie.h>
#include <Netcode/Network/NetworkErrorCode.h>
#include <Netcode/System/SecureString.h>
#include "Scripts/LocalPlayerWeaponScript.h"
#include <iomanip>
#include <openssl/rand.h>
#include <Netcode/Network/SslUtil.h>

class ServerConnRequestFilter : public nn::FilterBase {
public:
	nn::FilterResult Run(Ptr<nn::NetcodeService> service, Ptr<nn::DtlsRoute> route, Netcode::Timestamp timestamp, nn::ControlMessage& cm) override {
		const np::Control * peerControl = cm.control;
		if(peerControl->type() != np::CONNECT_REQUEST || !peerControl->has_connect_request()) {
			return nn::FilterResult::IGNORED;
		}
		
		const np::ConnectRequest * connReq = &peerControl->connect_request();

		if(route == nullptr || route->state != nn::DtlsRouteState::ESTABLISHED) {
			return nn::FilterResult::CONSUMED;
		}

		nn::ConnectionStorage* connections = service->GetConnections();

		if(connections->GetConnectionByEndpoint(cm.packet->endpoint) != nullptr) {
			return nn::FilterResult::CONSUMED;
		}

		std::string nonce = nn::GenerateNonce();

		if(nonce.empty()) {
			Log::Error("Failed to generate nonce");
		}

		// for now accept everything
		Ref<nn::NetAllocator> alloc = service->MakeAllocator(1024);
		np::Control * localControl = alloc->MakeProto<np::Control>();
		localControl->set_sequence(1);
		localControl->set_type(np::MessageType::CONNECT_RESPONSE);
		np::ConnectResponse * connResp = localControl->mutable_connect_response();
		connResp->set_type(connReq->type());
		connResp->set_current_map("mat_test_map");
		connResp->set_nonce(std::move(nonce));
		connResp->set_error_code(0);

		Ref<nn::ConnectionBase> conn = std::make_shared<Connection>(service->GetIOContext());
		conn->dtlsRoute = route;
		conn->pmtu = nn::MtuValue{ route->mtu };
		conn->endpoint = route->endpoint;
		conn->remoteGameSequence = 0;
		conn->localControlSequence = 1;
		conn->localGameSequence = 1;
		conn->remoteControlSequence = peerControl->sequence();

		nn::ControlMessage localCm;
		localCm.control = localControl;
		localCm.allocator = alloc;
		
		connections->AddConnection(conn);

		service->Send(alloc, alloc->MakeCompletionToken<nn::TrResult>(), conn->dtlsRoute, localCm, conn->endpoint, conn->pmtu, nn::ResendArgs{ 1000, 3 });
		
		return nn::FilterResult::CONSUMED;
	}
};

class ServerClockSyncRequestFilter : public nn::FilterBase {
public:
	nn::FilterResult Run(Ptr<nn::NetcodeService> service, Ptr<nn::DtlsRoute> route, Netcode::Timestamp timestamp, nn::ControlMessage & cm) override {
		/*
		np::Header * header = cm.header;
		Ref<nn::NetAllocator> alloc = cm.allocator;

		if(header->type() == np::MessageType::CLOCK_SYNC_REQUEST) {
			np::Header * h = alloc->MakeProto<np::Header>();
			h->set_sequence(header->sequence());
			h->unsafe_arena_set_allocated_time_sync(header->unsafe_arena_release_time_sync());
			h->set_type(np::CLOCK_SYNC_RESPONSE);
			np::TimeSync * ts = h->mutable_time_sync();
			ts->set_server_req_reception(Netcode::ConvertTimestampToUInt64(cm.packet->GetTimestamp()));
			ts->set_server_resp_transmission(Netcode::ConvertTimestampToUInt64(Netcode::SystemClock::LocalNow()));
			service->Send(std::move(alloc), h, cm.packet->GetEndpoint());
			return nn::FilterResult::CONSUMED;
		}*/

		return nn::FilterResult::IGNORED;
	}
};

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

	static int i = 0;

	if(graphics->debug != nullptr) {
		if(i++ > 0 && i % 16 == 0) {
			std::wostringstream woss;
			woss << "FPS: " << std::setw(7) << std::fixed << std::setprecision(2) << fpsCounter.GetAvgFramesPerSecond();
			fpsValue = woss.str();
		}

		graphics->debug->DrawDebugText(fpsValue, Netcode::Float2::Zero);
	} else {
		if(i++ > 0 && i % 1024 == 0) {
			//Log::Debug("FPS: {0}", fpsCounter.GetAvgFramesPerSecond(), 0.0);
		}
	}
	
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
	Service::Init<GameApp *>(this);

	pxService = Service::Get<Netcode::Physics::PhysX>();
	pxService->CreateResources();

	Service::Init<GameSceneManager>();

	gameScene = Service::Get<GameSceneManager>()->GetScene();

	gameScene->Setup();
}

static Netcode::ErrorCode ConvertUserData(const rapidjson::Value& v, UserData& outData) {
	if(!v.IsObject()) {
		return make_error_code(Netcode::JsonErrc::INVALID_VALUE);
	}

	UserData ud;
	
	if(const auto it = v.FindMember("id"); it != v.MemberEnd() && it->value.IsInt()) {
		ud.id = it->value.GetInt();
	} else return make_error_code(Netcode::JsonErrc::INVALID_VALUE);

	if(const auto it = v.FindMember("name"); it != v.MemberEnd() && it->value.IsString()) {
		ud.name = it->value.GetString();
	} else return make_error_code(Netcode::JsonErrc::INVALID_VALUE);

	if(const auto it = v.FindMember("is_banned"); it != v.MemberEnd() && it->value.IsBool()) {
		ud.isBanned = it->value.GetBool();
	} else return make_error_code(Netcode::JsonErrc::INVALID_VALUE);
	
	std::swap(ud, outData);

	return Netcode::ErrorCode{};
}

static Netcode::ErrorCode ConvertUserData(const std::string & inputString, UserData& outData) {
	rapidjson::Document doc;
	doc.Parse(inputString);

	if(doc.HasParseError()) {
		return make_error_code(static_cast<Netcode::JsonErrc>(doc.GetParseError()));
	}

	return ConvertUserData(doc, outData);
}

static Netcode::ErrorCode ConvertServerData(const rapidjson::Value& v, GameServerData & outData) {
	if(!v.IsObject()) {
		return make_error_code(Netcode::JsonErrc::INVALID_VALUE);
	}

	GameServerData gsd;

	if(const auto it = v.FindMember("hostname"); it != v.MemberEnd() && (it->value.IsString() || it->value.IsNull())) {
		if(it->value.IsString()) {
			gsd.hostname = it->value.GetString();
		}
	} else return make_error_code(Netcode::JsonErrc::INVALID_VALUE);

	if(const auto it = v.FindMember("owner"); it != v.MemberEnd() && it->value.IsString()) {
		gsd.host = it->value.GetString();
	} else return make_error_code(Netcode::JsonErrc::INVALID_VALUE);

	if(const auto it = v.FindMember("server_ip"); it != v.MemberEnd() && it->value.IsString()) {
		gsd.address = it->value.GetString();
	} else return make_error_code(Netcode::JsonErrc::INVALID_VALUE);

	if(const auto it = v.FindMember("control_port"); it != v.MemberEnd() && it->value.IsInt()) {
		gsd.port = it->value.GetInt();
	} else return make_error_code(Netcode::JsonErrc::INVALID_VALUE);

	if(const auto it = v.FindMember("max_players"); it != v.MemberEnd() && it->value.IsInt()) {
		gsd.availableSlots = it->value.GetInt();
	} else return make_error_code(Netcode::JsonErrc::INVALID_VALUE);

	if(const auto it = v.FindMember("active_players"); it != v.MemberEnd() && it->value.IsInt()) {
		gsd.activePlayers = it->value.GetInt();
	} else return make_error_code(Netcode::JsonErrc::INVALID_VALUE);

	gsd.rtt = -1;
	std::swap(gsd, outData);
	
	return Netcode::ErrorCode{};
}

static Netcode::ErrorCode ConvertServerData(const std::string & inputString, std::vector<GameServerData> & outData) {
	rapidjson::Document doc;
	doc.Parse(inputString);

	if(doc.HasParseError()) {
		return make_error_code(static_cast<Netcode::JsonErrc>(doc.GetParseError()));
	}

	if(!doc.IsArray()) {
		return make_error_code(Netcode::JsonErrc::INVALID_VALUE);
	}
	
	std::vector<GameServerData> vec;
	vec.reserve(doc.GetArray().Size());
	
	for(const auto & v : doc.GetArray()) {
		GameServerData gsd;

		Netcode::ErrorCode ec = ConvertServerData(v, gsd);

		if(ec) {
			return ec;
		}

		vec.emplace_back(std::move(gsd));
	}

	std::swap(outData, vec);

	return Netcode::ErrorCode{};
}

void GameApp::CreateUI() {
	enum PageEnum {
		E_LOGIN, E_MAIN, E_OPTIONS, E_SERVER_BROWSER, E_HOST_SERVER, E_HUD, E_LOADING
	};

	renderer.ui_Input = &pageManager;

	clientSession = std::dynamic_pointer_cast<nn::ClientSession>(network->CreateClient());
	playerConnection = std::make_shared<Connection>(clientSession->GetIOContext());

	serverSession = std::dynamic_pointer_cast<nn::ServerSession>(network->CreateServer());
	serverSession->Start();
	auto service = serverSession->GetService();
	service->AddFilter(std::make_unique<ServerConnRequestFilter>());
	service->AddFilter(std::make_unique<ServerClockSyncRequestFilter>());

	clientSession->Connect(playerConnection, "localhost", 8889)->Then([](const Netcode::ErrorCode& ec) -> void {
		Log::Debug("Done: {0}", ec.message());
	});

	return;

	
	Ref<HostServerPage> hostServerPage = pageManager.CreatePage<HostServerPage>(*pxService->physics);
	hostServerPage->InitializeComponents();
	pageManager.AddPage(hostServerPage);
	pageManager.NavigateTo(0);

	
	Ref<LoginPage> loginPage = pageManager.CreatePage<LoginPage>(*pxService->physics);
	Ref<MainMenuPage> mmPage = pageManager.CreatePage<MainMenuPage>(*pxService->physics);
	Ref<OptionsPage> optionsPage = pageManager.CreatePage<OptionsPage>(*pxService->physics);
	Ref<ServerBrowserPage> serverBrowserPage = pageManager.CreatePage<ServerBrowserPage>(*pxService->physics);
	Ref<HUD> hud = pageManager.CreatePage<HUD>(*pxService->physics);
	Ref<LoadingPage> loadingPage = pageManager.CreatePage<LoadingPage>(*pxService->physics);

	loginPage->InitializeComponents();
	mmPage->InitializeComponents();
	optionsPage->InitializeComponents();
	serverBrowserPage->InitializeComponents();
	hud->InitializeComponents();
	loadingPage->InitializeComponents();

	pageManager.AddPage(loginPage);
	pageManager.AddPage(mmPage);
	pageManager.AddPage(optionsPage);
	pageManager.AddPage(serverBrowserPage);
	pageManager.AddPage(hud);
	pageManager.AddPage(loadingPage);

	loginPage->onExitClick = [this]() -> void {
		mainThreadDispatcher.Post([this]() -> void {
			window->Shutdown();
		});
	};
	
	optionsPage->onBack = [this]()->void {
		mainThreadDispatcher.Post([this]() -> void {
			pageManager.ReturnToLastPage();
		});
	};

	mmPage->onExitClick = loginPage->onExitClick;

	mmPage->onLogoutClick = [this]() -> void {
		Netcode::IO::File::Delete(Netcode::Config::Get<std::wstring>(L"user.sessionFile:string"));
		network->EraseCookie("netcode-auth");
		
		mainThreadDispatcher.Post([this]() -> void {
			pageManager.NavigateWithoutHistory(E_LOGIN);
		});
	};

	mmPage->onOptionsClick = [this]() -> void {
		mainThreadDispatcher.Post([this]() -> void {
			pageManager.NavigateTo(E_OPTIONS);
		});
	};

	mmPage->onJoinGameClick = [this, loadp = loadingPage.get(), sbp = serverBrowserPage.get()]() -> void {
		mainThreadDispatcher.Post([this, loadp, sbp]() -> void {
			pageManager.NavigateTo(E_SERVER_BROWSER);
			pageManager.NavigateTo(E_LOADING);
			loadp->SetLoader(L"Fetching data...");

			network->QueryServers()->Then([this, loadp, sbp](const Netcode::Network::Response& resp) -> void {
				Netcode::SleepFor(std::chrono::milliseconds(750));
				
				Netcode::ErrorCode ec = resp.GetErrorCode();
				if(ec) {
					mainThreadDispatcher.Post([loadp, ec]() -> void {
						loadp->SetError(Netcode::Utility::ToWideString(Netcode::ErrorCodeToString(ec)));
					});
				}

				if(resp.result() != boost::beast::http::status::ok) {
					std::wstring err = L"Error(Netcode.Http#" + std::to_wstring(resp.result_int()) + L")";
					mainThreadDispatcher.Post([loadp, e = std::move(err)]() -> void {
						loadp->SetError(e);
					});
					return;
				}

				std::vector<GameServerData> serverData;
				
				ec = ConvertServerData(resp.body(), serverData);

				if(ec) {
					mainThreadDispatcher.Post([loadp, ec]() -> void {
						loadp->SetError(Netcode::Utility::ToWideString(Netcode::ErrorCodeToString(ec)));
					});
				} else {
					mainThreadDispatcher.Post([d = std::move(serverData), loadp, sbp]() mutable -> void {
						loadp->CloseDialog();
						sbp->SetList(std::move(d));
					});
				}
			});
		});
	};

	loginPage->onLoginClick = [this, lp = loginPage.get(), loadp = loadingPage.get()]() -> void {
		mainThreadDispatcher.Post([this, lp, loadp]() -> void {
			pageManager.NavigateTo(E_LOADING);
			
			if(lp->usernameTextBox->Text().empty() || lp->passwordTextBox->Text().empty()) {
				loadp->SetError(L"Both fields are required");
				return;
			}
			
			loadp->SetLoader(L"Please wait...");
			
			network->Login(lp->usernameTextBox->Text(), lp->passwordTextBox->Text())->Then([this, loadp](const Netcode::Network::Response & httpResp) -> void {
				Netcode::SleepFor(std::chrono::milliseconds(750));

				if(httpResp.result() != boost::beast::http::status::ok) {
					std::wostringstream woss;
					woss << L"Error(Netcode.Http#" << httpResp.result_int() << L"): ";

					if(httpResp.result() == boost::beast::http::status::client_closed_request) {
						if(httpResp.GetErrorCode()) {
							woss << Netcode::Utility::ToWideString(httpResp.GetErrorCode().message());
						} else {
							woss << L"Hostname resolution failed";
						}
					} else {
						woss << Netcode::Utility::ToWideString(std::string{ httpResp.reason() });
						woss << L", Authentication failed";
					}
					std::wstring err = woss.str();

					mainThreadDispatcher.Post([e = std::move(err), loadp]()->void {
						loadp->SetError(e);
					});

					return;
				}

				auto pair = httpResp.equal_range(boost::beast::http::field::set_cookie);

				for(auto it = pair.first; it != pair.second; it++) {
					nn::Cookie c;
					boost::beast::string_view sv = it->value();
					
					if(nn::Cookie::Parse(std::string_view{ sv.data(), sv.size() }, c)) {
						network->SetCookie(c);
					}
				}

				nn::Cookie cookie = network->GetCookie("netcode-auth");

				if(!cookie.IsValid()) {
					std::wstring err = Netcode::Utility::ToWideString(Netcode::ErrorCodeToString(make_error_code(Netcode::NetworkErrc::BAD_COOKIE)));
					mainThreadDispatcher.Post([e = std::move(err), loadp]() -> void {
						loadp->SetError(e);
					});
					return;
				}
				Netcode::SecureString ss;
				ss.Store(Netcode::Utility::ToWideString(cookie.GetValue()), L"netcode-session");
				Netcode::Config::Set<Netcode::SecureString>(L"user.session", ss);

				try {
					Netcode::IO::File cookieFile{ Netcode::Config::Get<std::wstring>(L"user.sessionFile:string") };
					Netcode::IO::FileWriter<Netcode::IO::File> writer{ cookieFile };
					writer->Write(ss.GetView());
					Log::Debug("Cookie was saved");
				} catch(Netcode::ExceptionBase& e) {
					// an error is fine here, will just prompt a new login at the next start
					Log::Error("Exception while trying to save cookie: {0}", e.ToString());
				}

				mainThreadDispatcher.Post([this, loadp]() -> void {
					loadp->CloseDialog();
					loadp->rootPanel->OnAnimationsFinished.Subscribe([this](Netcode::UI::Control *)->void {
						pageManager.NavigateTo(E_MAIN);
					});
				});
			});

		});
	};

	serverBrowserPage->onRefresh = [this, loadp = loadingPage.get(), sbp = serverBrowserPage.get()]() -> void {
		mainThreadDispatcher.Post([this, loadp, sbp]() -> void {
			pageManager.NavigateTo(E_SERVER_BROWSER);
			pageManager.NavigateTo(E_LOADING);
			loadp->SetLoader(L"Fetching data...");

			network->QueryServers()->Then([this, loadp, sbp](const Netcode::Network::Response & resp) -> void {
				Netcode::SleepFor(std::chrono::milliseconds(750));

				Netcode::ErrorCode ec = resp.GetErrorCode();
				if(ec) {
					mainThreadDispatcher.Post([loadp, ec]() -> void {
						loadp->SetError(Netcode::Utility::ToWideString(Netcode::ErrorCodeToString(ec)));
					});
				}

				if(resp.result() != boost::beast::http::status::ok) {
					std::wstring err = L"Error(Netcode.Http#" + std::to_wstring(resp.result_int()) + L")";
					mainThreadDispatcher.Post([loadp, e = std::move(err)]() -> void {
						loadp->SetError(e);
					});
					return;
				}

				std::vector<GameServerData> serverData;

				ec = ConvertServerData(resp.body(), serverData);

				if(ec) {
					mainThreadDispatcher.Post([loadp, ec]() -> void {
						loadp->SetError(Netcode::Utility::ToWideString(Netcode::ErrorCodeToString(ec)));
					});
				} else {
					mainThreadDispatcher.Post([d = std::move(serverData), loadp, sbp]() mutable -> void {
						loadp->CloseDialog();
						sbp->SetList(std::move(d));
					});
				}
			});
		});
	};

	serverBrowserPage->onJoinCallback = [this, loadp = loadingPage.get()](const GameServerData * gsd) -> void {
		if(gsd == nullptr) {
			return;
		}

		mainThreadDispatcher.Post([this, gsd, loadp]() -> void {
			if(clientSession == nullptr) {
				clientSession = std::dynamic_pointer_cast<nn::ClientSession>(network->CreateClient());
				playerConnection = std::make_shared<Connection>(clientSession->GetIOContext());
			}

			std::string h = (!gsd->hostname.empty()) ? gsd->hostname : gsd->address;
			std::wostringstream woss;
			woss << "Connecting to " << Netcode::Utility::ToWideString(h) << L"...";

			pageManager.NavigateTo(E_LOADING);
			loadp->SetLoader(woss.str());

			clientSession->Connect(playerConnection, std::move(h), gsd->port)->Then([loadp](const Netcode::ErrorCode & ec) -> void {
				if(ec) {
					loadp->SetError(Netcode::Utility::ToWideString(Netcode::ErrorCodeToString(ec)));
				} else {
					loadp->CloseDialog();
				}
			});
			
		});
	};
	
	serverBrowserPage->onCancel = [this]() -> void {
		mainThreadDispatcher.Post([this]() -> void {
			pageManager.ReturnToLastPage();
		});
	};

	pageManager.NavigateTo(E_LOGIN);
	renderer.ui_Input = &pageManager;

	auto session = Netcode::Config::Get<Netcode::SecureString>(L"user.session");
	if(!session.Empty()) {
		std::wstring sessionValue = session.Load();
		if(!sessionValue.empty()) {
			std::string cookieStr = "netcode-auth=" + Netcode::Utility::ToNarrowString(sessionValue);
			Netcode::Network::Cookie c;

			if(!Netcode::Network::Cookie::Parse(cookieStr, c)) {
				Log::Error("Unexpected error while parsing cookie");
			}
			
			if(Netcode::Network::Cookie::Parse(cookieStr, c)) {
				pageManager.NavigateTo(E_LOADING);
				loadingPage->SetLoader(L"Authenticating...");
				network->SetCookie(c);
				network->Status()->Then([this](const Netcode::Network::Response & resp) -> void {
					Netcode::SleepFor(std::chrono::milliseconds(750));
					if(resp.GetErrorCode() || resp.result() != boost::beast::http::status::ok || ConvertUserData(resp.body(), user)) {
						Log::Info("Failed to authenticate by session");
						mainThreadDispatcher.Post([this]() -> void {
							pageManager.NavigateWithoutHistory(E_LOGIN);
						});
					} else {
						mainThreadDispatcher.Post([this]() -> void {
							pageManager.NavigateWithoutHistory(E_MAIN);
						});
					}
				});
			}
		}
	}
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

	Netcode::Input::OnKeyPressed->Subscribe([this](Netcode::Key key, Netcode::KeyModifiers modifiers) -> void {
		if(modifiers == Netcode::KeyModifier::NONE) {
			if(key == Netcode::KeyCode::F5) {
				ReloadMap();
				Log::Debug("Map reloaded");
			}

			if(key == Netcode::KeyCode::F9) {
				static uint8_t message[1024];
				static uint8_t dst[1500];
				memset(message, 'A', sizeof(message));
				Netcode::MutableArrayView<uint8_t> dstV { dst, sizeof(dst) };
				Netcode::ErrorCode ec = nn::SslSend(playerConnection->dtlsRoute->ssl.get(), dstV, Netcode::ArrayView<uint8_t>{ message, sizeof(message) });

				if(ec) {
					
				} else {
					BIO * bio = BIO_new_mem_buf(dstV.Data(), dstV.Size());
					clientSession->GetService()->Send(nullptr, nullptr, playerConnection->endpoint, nn::ssl_ptr<BIO>{ bio }, 1280);
					Log::Debug("Punch it chewy");
				}
				
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
	Netcode::Initialize();
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



