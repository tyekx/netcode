#pragma once

#include <Netcode/UI/Page.h>

#include "Services.h"
#include "Asset.h"
#include <array>

enum PagesEnum {
	LOGIN_PAGE,
	SERVER_BROWSER_PAGE,
	LOADING_PAGE,
	MAIN_MENU
};

namespace Netcode::UI {
	class Button;
	class TextBox;
	class Panel;
	class Label;
	class Slider;
};

class PageBase : public Netcode::UI::Page {
protected:
	constexpr static Netcode::Float4 COLOR_ACCENT{ 1.0f, 0.4f, 0.533333f, 1.0f };
	constexpr static Netcode::Float4 COLOR_SECONDARY{ 0.043137f, 0.164705f, 0.247058f, 0.95f };
	constexpr static Netcode::Float4 COLOR_TERTIARY{ 0.0746f, 0.2706f, 0.4314f, 0.90f };
	constexpr static Netcode::Float4 COLOR_BORDER{ 0.02745f, 0.141176f, 0.211764f, 1.0f };
	constexpr static Netcode::Float4 COLOR_HOVER{ 0.458823f, 0.235294f, 0.282353f, 1.0f };
	constexpr static Netcode::Float4 COLOR_TEXT{ 0.8f, 0.8f, 0.8f, 1.0f };
	
	constexpr static float BORDER_RADIUS = 4.0f;
	constexpr static float BORDER_WIDTH = 2.0f;

	Ref<Netcode::SpriteFont> textFont;

	Ref<Netcode::UI::Button> CreateButton(const wchar_t * text);

	Ref<Netcode::UI::TextBox> CreateTextBox();

	Ref<Netcode::UI::Slider> CreateSliderWithTextBox();
public:
	using Netcode::UI::Page::Page;

	void DebugDump();

	virtual void InitializeComponents() override;
};

class LoginPage : public PageBase {
	Ref<Netcode::GpuResource> aenami;
public:
	using PageBase::PageBase;

	std::function<void()> onLoginClick;
	std::function<void()> onExitClick;

	Ref<Netcode::UI::TextBox> usernameTextBox;
	Ref<Netcode::UI::TextBox> passwordTextBox;
	Ref<Netcode::UI::Button> loginButton;
	Ref<Netcode::UI::Button> exitButton;

	virtual void InitializeComponents() override;

	virtual void Activate() override;
};

class MainMenuPage : public PageBase {
public:
	using PageBase::PageBase;

	Ref<Netcode::UI::Label> loggedInLabel;
	Ref<Netcode::UI::Button> joinGameBtn;
	Ref<Netcode::UI::Button> createGameBtn;
	Ref<Netcode::UI::Button> optionsBtn;
	Ref<Netcode::UI::Button> logoutBtn;
	Ref<Netcode::UI::Button> exitBtn;

	std::function<void()> onJoinGameClick;
	std::function<void()> onCreateGameClick;
	std::function<void()> onOptionsClick;
	std::function<void()> onLogoutClick;
	std::function<void()> onExitClick;

	std::string currentlyDisplayedUsername;
	
	void SetUsername(const std::string & username);

	virtual void Activate() override;
	
	virtual void InitializeComponents() override;
};

class ServerBrowserPage : public PageBase {

	Ref<Control> CreateServerRow(const GameServerData & srvData, int32_t index);

	Ref<Control> listControl;
	std::vector<GameServerData> serverData;
	int32_t selectionIndex;

	
public:
	std::function<void()> onCancel;
	std::function<void()> onRefresh;
	std::function<void(const GameServerData *)> onJoinCallback;
	
	using PageBase::PageBase;

	virtual void InitializeComponents() override;

	void SetList(std::vector<GameServerData> srvData);
};

class LoadingPage : public PageBase {
	Ref<Netcode::GpuResource> loadingIcon;
	Ref<Netcode::GpuResource> warningIcon;

	enum class DialogState {
		CLOSED, LOADER, ERROR_MSG, DIALOG
	};

	Ref<Control> content;
	Ref<Control> loaderRoot;
	Ref<Netcode::UI::Panel> loaderIconPanel;
	Ref<Netcode::UI::Label> loaderLabel;

	Ref<Control> errorRoot;
	Ref<Netcode::UI::Panel> errorIconPanel;
	Ref<Netcode::UI::Label> errorLabel;
	Ref<Netcode::UI::Button> errorButton;
	
	DialogState state;

	void StartShowLoaderAnims(float delay);
	void StartHideLoaderAnims(float delay);

	void StartShowErrorAnims(float delay);
	void StartHideErrorAnims(float delay);
	

public:

	Ref<Netcode::UI::Control> rootPanel;
	
	using PageBase::PageBase;

	void Destruct() override;
	
	void CloseDialog();

	virtual void InitializeComponents() override;

	virtual void Activate() override;

	virtual void Deactivate() override;

	void SetError(const std::wstring & msg);

	void SetDialog(const std::wstring & msg) {

	}

	void SetLoader(const std::wstring & msg);
};

class HUD : public PageBase {
	Ref<Netcode::GpuResource> crosshairTexture;
	Ref<Netcode::GpuResource> hitpipTexture;

	Ref<Netcode::UI::Panel> killFeed;
	uint32_t numActiveLog;
	uint32_t maxLogCount;

	struct LogEntry {
		Ptr<Netcode::UI::Label> label;
		float displayedFor;
	};
	
	std::array<LogEntry, 8> logBuffer;
	
public:
	using PageBase::PageBase;

	virtual void Update(float dt) override;

	void AddKillFeedItem(const std::wstring & text);
	
	virtual void InitializeComponents() override;
};

class OptionsPage : public PageBase {
public:
	using PageBase::PageBase;

	std::function<void()> onBack;
	
	virtual void InitializeComponents() override;
};
