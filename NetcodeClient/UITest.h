#pragma once

#include <Netcode/UI/Page.h>

#include "Services.h"

enum PagesEnum {
	LOGIN_PAGE,
	SERVER_BROWSER_PAGE,
	LOADING_PAGE,
	MAIN_MENU
};

namespace Netcode::UI {
	class Button;
	class TextBox;
};

class PageBase : public Netcode::UI::Page {
protected:
	constexpr static Netcode::Float4 COLOR_ACCENT{ 1.0f, 0.4f, 0.533333f, 1.0f };
	constexpr static Netcode::Float4 COLOR_SECONDARY{ 0.043137f, 0.164705f, 0.247058f, 0.8f };
	constexpr static Netcode::Float4 COLOR_BORDER{ 0.02745f, 0.141176f, 0.211764f, 1.0f };
	constexpr static Netcode::Float4 COLOR_HOVER{ 0.458823f, 0.235294f, 0.282353f, 1.0f };
	constexpr static Netcode::Float4 COLOR_TEXT{ 0.8f, 0.8f, 0.8f, 1.0f };

	constexpr static float BORDER_RADIUS = 4.0f;
	constexpr static float BORDER_WIDTH = 2.0f;

	Ref<Netcode::SpriteFont> textFont;

	Ref<Netcode::UI::Button> CreateButton(const wchar_t * text);

	Ref<Netcode::UI::TextBox> CreateTextBox();
public:
	using Netcode::UI::Page::Page;

	void DebugDump();

	virtual void InitializeComponents() override;
};

class LoginPage : public PageBase {
	Ref<Netcode::GpuResource> aenami;
public:
	using PageBase::PageBase;

	virtual void InitializeComponents() override;
};

class ServerBrowserPage : public PageBase {

	Ref<Netcode::UI::Control> CreateServerRow(std::wstring serverIp);

public:
	using PageBase::PageBase;

	virtual void InitializeComponents() override;
};

class LoadingPage : public PageBase {
	Ref<Netcode::GpuResource> loadingIcon;
	Ref<Netcode::GpuResource> warningIcon;

	Ref<Netcode::UI::Control> rootPanel;

	Ref<Netcode::UI::Control> errorContent;
	Ref<Netcode::UI::Control> dialogContent;
	Ref<Netcode::UI::Control> loaderContent;

	void CloseDialog();

public:
	using PageBase::PageBase;

	virtual void InitializeComponents() override;

	virtual void Activate() override;

	void SetError(const std::wstring & msg);

	void SetDialog(const std::wstring & msg) {

	}

	void SetLoader(const std::wstring & msg) {

	}
};


