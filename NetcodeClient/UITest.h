#pragma once

#include <Netcode/UI/Page.h>

#include "Services.h"

enum PagesEnum {
	LOGIN_PAGE,
	MAIN_MENU,
	LOADING_PAGE
};

namespace Netcode::UI {
	class Button;
	class TextBox;
};

class LoginPage : public Netcode::UI::Page {
	Netcode::GpuResourceRef aenami;
	Netcode::GpuResourceRef loadingIcon;

	Netcode::SpriteFontRef textFont;

public:
	constexpr static Netcode::Float4 COLOR_ACCENT{ 1.0f, 0.4f, 0.533333f, 1.0f };
	constexpr static Netcode::Float4 COLOR_SECONDARY{ 0.043137f, 0.164705f, 0.247058f, 0.8f };
	constexpr static Netcode::Float4 COLOR_BORDER{ 0.02745f, 0.141176f, 0.211764f, 1.0f };
	constexpr static Netcode::Float4 COLOR_HOVER{ 0.458823f, 0.235294f, 0.282353f, 1.0f };
	constexpr static Netcode::Float4 COLOR_TEXT{ 0.8f, 0.8f, 0.8f, 1.0f };

	constexpr static float BORDER_RADIUS = 4.0f;
	constexpr static float BORDER_WIDTH = 2.0f;

	using Netcode::UI::Page::Page;

	std::shared_ptr<Netcode::UI::Button> CreateButton(const wchar_t * text);

	std::shared_ptr<Netcode::UI::TextBox> CreateTextBox();

	virtual ~LoginPage() = default;

	virtual void InitializeComponents() override;


};


