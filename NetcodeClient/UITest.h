#pragma once

#include <Netcode/UI/Page.h>

#include "Services.h"

enum PagesEnum {
	LOGIN_PAGE,
	SERVER_BROWSER_PAGE,
	MAIN_MENU,
	LOADING_PAGE
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

	Netcode::SpriteFontRef textFont;

	std::shared_ptr<Netcode::UI::Button> CreateButton(const wchar_t * text);

	std::shared_ptr<Netcode::UI::TextBox> CreateTextBox();
public:
	using Netcode::UI::Page::Page;

	void DebugDump();

	virtual void InitializeComponents() override;
};

class LoginPage : public PageBase {
	Netcode::GpuResourceRef aenami;
	Netcode::GpuResourceRef loadingIcon;
public:
	using PageBase::PageBase;

	virtual void InitializeComponents() override;
};

class ServerBrowserPage : public PageBase {

	std::shared_ptr<Netcode::UI::Control> CreateServerRow(std::wstring serverIp);

public:
	using PageBase::PageBase;

	virtual void InitializeComponents() override;
};


