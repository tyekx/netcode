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

	DirectX::XMFLOAT2 loginPageSize;

	UITextBox usernameTextBox;
	UITextBox passwordTextBox;

	Egg::ResourceViewsRef loadIconView;
	DirectX::XMUINT2 loadIconTextureSize;
	Egg::ResourceViewsRef btnBackgroundView;
	DirectX::XMUINT2 btnBackgroundTextureSize;
	Egg::ResourceViewsRef textBoxBackgroundView;
	DirectX::XMUINT2 textBoxBackgroundTextureSize;
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

		spriteFontBuilder->LoadFont(L"titillium24.spritefont");
		font24 = spriteFontBuilder->Build();

		spriteFontBuilder->LoadFont(L"titillium48bold.spritefont");
		font48 = spriteFontBuilder->Build();

		loadIconView = app->graphics->resources->CreateShaderResourceViews(1);
		btnBackgroundView = app->graphics->resources->CreateShaderResourceViews(1);
		textBoxBackgroundView = app->graphics->resources->CreateShaderResourceViews(1);

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

		loadPage = scene->CreatePage();
		loginPage = scene->CreatePage();
		mainPage = scene->CreatePage();

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
	}

	virtual void OnResized(int width, int height) override {
		DirectX::XMFLOAT2 wh{ static_cast<float>(width), static_cast<float>(height) };

		loginPage.SetPosition(DirectX::XMFLOAT2{
			(wh.x - loginPageSize.x) / 2.0f,
			(wh.y - loginPageSize.y) / 2.0f
		});
	}

	virtual void Activate() override {
		loginPage.Show();
	}

	virtual void Deactivate() override {
		loginPage.Hide();
	}

};
