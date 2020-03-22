#pragma once

#include "UIScene.h"
#include <Egg/Modules.h>

class Layer : public Egg::Module::TAppEventHandler {
public:
	virtual ~Layer() = default;

	virtual void Construct(Egg::Module::AApp * app) = 0;
	virtual void Activate() = 0;
	virtual void Deactivate() = 0;
};

using Egg::Graphics::ResourceState;

class MainMenuLayer : public Layer {
	UIScene * scene;

	UIPagePrefab loadPage;
	UIPagePrefab loginPage;
	UIPagePrefab mainPage;
	UIPagePrefab playPage;

	DirectX::XMFLOAT2 loadPageSize;
	DirectX::XMFLOAT2 loginPageSize;
	DirectX::XMFLOAT2 mainPageSize;
	DirectX::XMFLOAT2 playPageSize;

	UITextBox usernameTextBox;
	UITextBox passwordTextBox;

	UISpinner spinner;
	UILabel loadingLabel;

	Egg::ResourceViewsRef loadIconView;
	DirectX::XMUINT2 loadIconTextureSize;
	Egg::ResourceViewsRef btnBackgroundView;
	DirectX::XMUINT2 btnBackgroundTextureSize;
	Egg::ResourceViewsRef textBoxBackgroundView;
	DirectX::XMUINT2 textBoxBackgroundTextureSize;
	Egg::ResourceViewsRef serversPanelBackgroundView;
	DirectX::XMUINT2 serversPanelBackgroundSize;
	Egg::SpriteFontRef font24;
	Egg::SpriteFontRef font48;
public:
	MainMenuLayer() = default;

	MainMenuLayer(UIScene * scene) : MainMenuLayer() {
		this->scene = scene;
	}

	virtual void Construct(Egg::Module::AApp* app) override {
		Egg::Graphics::UploadBatch uploadBatch;
		
		Egg::TextureBuilderRef textureBuilder = app->graphics->CreateTextureBuilder();
		Egg::SpriteFontBuilderRef spriteFontBuilder = app->graphics->CreateSpriteFontBuilder();

		textureBuilder->LoadTexture2D(L"btn_background.png");
		Egg::TextureRef btnBackgroundTexture = textureBuilder->Build();

		textureBuilder->LoadTexture2D(L"textbox_background.png");
		Egg::TextureRef textBoxBackgroundTexture = textureBuilder->Build();

		textureBuilder->LoadTexture2D(L"loading_icon.png");
		Egg::TextureRef loadingIconTexture = textureBuilder->Build();

		textureBuilder->LoadTexture2D(L"server_panel_background.png");
		Egg::TextureRef serverPanelBackgroundTexture = textureBuilder->Build();

		spriteFontBuilder->LoadFont(L"titillium24.spritefont");
		font24 = spriteFontBuilder->Build();

		spriteFontBuilder->LoadFont(L"titillium48bold.spritefont");
		font48 = spriteFontBuilder->Build();

		loadIconView = app->graphics->resources->CreateShaderResourceViews(1);
		btnBackgroundView = app->graphics->resources->CreateShaderResourceViews(1);
		textBoxBackgroundView = app->graphics->resources->CreateShaderResourceViews(1);
		serversPanelBackgroundView = app->graphics->resources->CreateShaderResourceViews(1);

		{
			const Egg::Image * loadingIconImg = loadingIconTexture->GetImage(0, 0, 0);
			loadIconTextureSize = DirectX::XMUINT2{ static_cast<uint32_t>(loadingIconImg->width), static_cast<uint32_t>(loadingIconImg->height) };
			uint64_t loadingIconTextureHandle = app->graphics->resources->CreateTexture2D(loadingIconImg);
			uploadBatch.Upload(loadingIconTextureHandle, loadingIconTexture);
			uploadBatch.ResourceBarrier(loadingIconTextureHandle, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
			loadIconView->CreateSRV(0, loadingIconTextureHandle);
		}

		{
			const Egg::Image * textBoxBackgroundImg = textBoxBackgroundTexture->GetImage(0, 0, 0);
			textBoxBackgroundTextureSize = DirectX::XMUINT2{ static_cast<uint32_t>(textBoxBackgroundImg->width), static_cast<uint32_t>(textBoxBackgroundImg->height) };
			uint64_t textBoxBackgroundTextureHandle = app->graphics->resources->CreateTexture2D(textBoxBackgroundImg);
			uploadBatch.Upload(textBoxBackgroundTextureHandle, textBoxBackgroundTexture);
			uploadBatch.ResourceBarrier(textBoxBackgroundTextureHandle, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
			textBoxBackgroundView->CreateSRV(0, textBoxBackgroundTextureHandle);
		}

		{
			const Egg::Image * btnBackgroundImg = btnBackgroundTexture->GetImage(0, 0, 0);
			btnBackgroundTextureSize = DirectX::XMUINT2{ static_cast<uint32_t>(btnBackgroundImg->width), static_cast<uint32_t>(btnBackgroundImg->height) };
			uint64_t btnBackgroundTextureHandle = app->graphics->resources->CreateTexture2D(btnBackgroundImg);
			uploadBatch.Upload(btnBackgroundTextureHandle, btnBackgroundTexture);
			uploadBatch.ResourceBarrier(btnBackgroundTextureHandle, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
			btnBackgroundView->CreateSRV(0, btnBackgroundTextureHandle);
		}

		app->graphics->frame->SyncUpload(uploadBatch);

		Egg::Graphics::UploadBatch ub2;

		{
			const Egg::Image * serversBackgroundImg = serverPanelBackgroundTexture->GetImage(0, 0, 0);
			serversPanelBackgroundSize = DirectX::XMUINT2{ static_cast<uint32_t>(serversBackgroundImg->width), static_cast<uint32_t>(serversBackgroundImg->height) };
			uint64_t serverPanelTextureHandle = app->graphics->resources->CreateTexture2D(serversBackgroundImg);
			ub2.Upload(serverPanelTextureHandle, serverPanelBackgroundTexture);
			ub2.ResourceBarrier(serverPanelTextureHandle, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
			serversPanelBackgroundView->CreateSRV(0, serverPanelTextureHandle);
		}

		app->graphics->frame->SyncUpload(ub2);

		loadPage = scene->CreatePage();
		loginPage = scene->CreatePage();
		mainPage = scene->CreatePage();
		playPage = scene->CreatePage();

		spinner = scene->CreateSpinner();
		spinner.SetIcon(loadIconView, loadIconTextureSize);
		spinner.SetSize(DirectX::XMFLOAT2{ 128.0f, 128.0f });
		spinner.SetPosition(DirectX::XMFLOAT2{ 100.0f, 0.0f });

		loadingLabel = scene->CreateLabel(L"Connecting...", font24, DirectX::XMFLOAT2{ 0.0f, 0.0f });
		loadingLabel.SetPosition(DirectX::XMFLOAT2{ 0.0f, 148.0f });
		loadingLabel.SetSize(DirectX::XMFLOAT2{ 328.0f, 48.0f });
		loadingLabel.SetTextAlignment(HorizontalAnchor::CENTER);

		loadPage.AddControl(spinner);
		loadPage.AddControl(loadingLabel);
		loadPageSize.x = 328.0f;
		loadPageSize.y = 200.0f;
		loadPage.SetSize(loadPageSize.x, loadPageSize.y);
		loadPage.Hide();

		UIButtonPrefab loginBtn = scene->CreateButton(L"Login",
			DirectX::XMFLOAT2{ 192.0f, 48.0f },
			DirectX::XMFLOAT2{ 198.0f, 112.0f + 100.0f },
			0.0f, font24, btnBackgroundView, btnBackgroundTextureSize);

		UIButtonPrefab exitBtn = scene->CreateButton(L"Exit",
			DirectX::XMFLOAT2{ 192.0f, 48.0f },
			DirectX::XMFLOAT2{ 0.0f, 112.0f + 100.0f },
			0.0f, font24, btnBackgroundView, btnBackgroundTextureSize);

		exitBtn.OnClick([app]() -> void {
			app->window->Shutdown();
		});

		loginBtn.OnClick([&]() -> void {
			loginPage.Hide();
			mainPage.Show();
		});

		usernameTextBox = scene->CreateTextBox(DirectX::XMFLOAT2{ 390.0f, 48.0f }, DirectX::XMFLOAT2{ 0.0f, 0.0f + 100.0f }, font24, textBoxBackgroundView, textBoxBackgroundTextureSize);
		usernameTextBox.SetPlaceholder(L"username");
		passwordTextBox = scene->CreateTextBox(DirectX::XMFLOAT2{ 390.0f, 48.0f }, DirectX::XMFLOAT2{ 0.0f, 54.0f + 100.0f }, font24, textBoxBackgroundView, textBoxBackgroundTextureSize);
		passwordTextBox.SetPasswordFlag();
		passwordTextBox.SetPlaceholder(L"password");

		UILabel netcodeLabel = scene->CreateLabel(L"Netcode", font48, DirectX::XMFLOAT2{ 0.0f, 0.0f });
		netcodeLabel.SetSize(DirectX::XMFLOAT2{ 390.0f, 64.0f });
		netcodeLabel.SetTextAlignment(HorizontalAnchor::CENTER);

		loginPage.AddControl(loginBtn);
		loginPage.AddControl(exitBtn);
		loginPage.AddControl(usernameTextBox);
		loginPage.AddControl(passwordTextBox);
		loginPage.AddControl(netcodeLabel);
		loginPageSize.x = 390.0f;
		loginPageSize.y = 360.0f;
		loginPage.SetSize(loginPageSize.x, loginPageSize.y);

		UILabel menuLabel = scene->CreateLabel(L"Menu", font48, DirectX::XMFLOAT2{ 0.0f, 0.0f });
		menuLabel.SetSize(DirectX::XMFLOAT2{ 256.0f, 64.0f });
		menuLabel.SetTextAlignment(HorizontalAnchor::CENTER);

		UIButtonPrefab playBtn = scene->CreateButton(L"Play",
			DirectX::XMFLOAT2{ 256.0f, 64.0f },
			DirectX::XMFLOAT2{ 0, 100.0f },
			0.0f, font24, btnBackgroundView, btnBackgroundTextureSize);

		UIButtonPrefab optionsBtn = scene->CreateButton(L"Options",
			DirectX::XMFLOAT2{ 256.0f, 64.0f },
			DirectX::XMFLOAT2{ 0, 64.0f + 108.0f },
			0.0f, font24, btnBackgroundView, btnBackgroundTextureSize);

		UIButtonPrefab mainExitBtn = scene->CreateButton(L"Exit",
			DirectX::XMFLOAT2{ 256.0f, 64.0f },
			DirectX::XMFLOAT2{ 0.0f, 128.0f + 116.0f },
			0.0f, font24, btnBackgroundView, btnBackgroundTextureSize);

		mainExitBtn.OnClick([app]() -> void {
			app->window->Shutdown();
		});

		UILabel playLabel = scene->CreateLabel(L"Play", font48, DirectX::XMFLOAT2{ 0.0f, 0.0f });
		playLabel.SetSize(DirectX::XMFLOAT2{ 590.0f, 100.0f });
		playLabel.SetTextAlignment(HorizontalAnchor::CENTER);

		UIButtonPrefab playBackBtn = scene->CreateButton(L"Back",
				DirectX::XMFLOAT2{ 192.0f, 48.0f },
				DirectX::XMFLOAT2{ 0.0f, 394.0f },
				0.0f, font24, btnBackgroundView, btnBackgroundTextureSize);

		UIButtonPrefab playRefreshBtn = scene->CreateButton(L"Refresh",
			DirectX::XMFLOAT2{ 192.0f, 48.0f },
			DirectX::XMFLOAT2{ 198.0f, 394.0f },
			0.0f, font24, btnBackgroundView, btnBackgroundTextureSize);

		UIButtonPrefab playJoinBtn = scene->CreateButton(L"Join",
			DirectX::XMFLOAT2{ 192.0f, 48.0f },
			DirectX::XMFLOAT2{ 398.0f, 394.0f },
			0.0f, font24, btnBackgroundView, btnBackgroundTextureSize);

		UIObject * panelTest=  scene->Create();
		panelTest->AddComponent<Transform>()->position = DirectX::XMFLOAT3{ 0.0f, 100.0f, 0.0f };

		UIElement * uiEl = panelTest->AddComponent<UIElement>();
		uiEl->width = 590.0f;
		uiEl->height = 288.0f;
		
		Sprite * sp = panelTest->AddComponent<Sprite>();
		sp->texture = serversPanelBackgroundView;
		sp->textureSize = serversPanelBackgroundSize;
		sp->diffuseColor = DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f };

		panelTest->SetActivityFlag(true);
		scene->Spawn(panelTest);

		mainPage.AddControl(menuLabel);
		mainPage.AddControl(playBtn);
		mainPage.AddControl(optionsBtn);
		mainPage.AddControl(mainExitBtn);
		mainPageSize.x = 256.0f;
		mainPageSize.y = 308.0f;
		mainPage.SetSize(mainPageSize.x, mainPageSize.y);
		mainPage.Hide();

		playPage.AddControl(playLabel);
		playPage.AddControl(playJoinBtn);
		playPage.AddControl(playRefreshBtn);
		playPage.AddControl(playBackBtn);
		playPageSize.x = 590.0f;
		playPageSize.y = 436.0f;
		playPage.SetSize(playPageSize.x, playPageSize.y);
		panelTest->Parent(playPage.GetRoot());

		loginPage.Hide();
	}

	virtual void OnResized(int width, int height) override {
		DirectX::XMFLOAT2 wh{ static_cast<float>(width), static_cast<float>(height) };

		loginPage.SetPosition(DirectX::XMFLOAT2{
			(wh.x - loginPageSize.x) / 2.0f,
			(wh.y - loginPageSize.y) / 2.0f
		});

		loadPage.SetPosition(DirectX::XMFLOAT2{
			(wh.x - loadPageSize.x) / 2.0f,
			(wh.y - loadPageSize.y) / 2.0f
		});

		mainPage.SetPosition(DirectX::XMFLOAT2{
			(wh.x - mainPageSize.x) / 2.0f,
			(wh.y - mainPageSize.y) / 2.0f
		});

		playPage.SetPosition(DirectX::XMFLOAT2{
			(wh.x - playPageSize.x) / 2.0f,
			0.0f
		});
	}

	virtual void Activate() override {
		//loginPage.Show();
	}

	virtual void Deactivate() override {
		//loginPage.Hide();
	}

};
