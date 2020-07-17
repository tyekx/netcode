#pragma once

#include "UI/Page.h"

#include "Services.h"

enum PagesEnum {
	LOGIN_PAGE,
	MAIN_MENU,
	LOADING_PAGE
};

class LoginPage : public UI::Page {
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

	using UI::Page::Page;

	std::shared_ptr<UI::Button> CreateButton(const wchar_t* text) {
		std::shared_ptr<UI::Button> button = std::make_shared<UI::Button>(eventAllocator, CreatePhysxActor());
		button->Sizing(UI::SizingType::FIXED);
		button->Size(Netcode::Float2{ 156.0f, 48.0f });
		button->BackgroundColor(COLOR_SECONDARY);
		button->BorderColor(COLOR_HOVER);
		button->BorderRadius(BORDER_RADIUS);
		button->BorderWidth(BORDER_WIDTH);
		button->Font(textFont);
		button->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		button->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);
		button->Text(text);
		button->TextColor(COLOR_ACCENT);
		return button;
	}

	std::shared_ptr<UI::TextBox> CreateTextBox() {
		std::shared_ptr<UI::TextBox> textBox = std::make_shared<UI::TextBox>(eventAllocator, CreatePhysxActor());
		textBox->Sizing(UI::SizingType::FIXED);
		textBox->Size(Netcode::Float2{ 280.0f, 48.0f });
		textBox->BackgroundColor(COLOR_SECONDARY);
		textBox->BorderColor(COLOR_HOVER);
		textBox->BorderRadius(BORDER_RADIUS);
		textBox->BorderWidth(BORDER_WIDTH);
		textBox->Font(textFont);
		textBox->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		textBox->HorizontalContentAlignment(UI::HorizontalAnchor::LEFT);
		textBox->Text(L"testtex");

		textBox->OnMouseEnter.Subscribe([](UI::Control * controlPointer, UI::MouseEventArgs & args) -> void {
			auto * pThis = static_cast<UI::Panel *>(controlPointer);
			pThis->AddAnimation(UI::MakeAnimation(
				pThis,
				&UI::Panel::BackgroundColor,
				&UI::Panel::BackgroundColor,
				UI::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_HOVER },
				UI::PlayOnceBehaviour{},
				&Netcode::Function::EaseOutQuad,
				0.7f
			));
		});

		textBox->OnMouseLeave.Subscribe([](UI::Control * controlPointer, UI::MouseEventArgs & args) -> void {
			auto * pThis = static_cast<UI::Panel *>(controlPointer);
			pThis->AddAnimation(UI::MakeAnimation(
				pThis,
				&UI::Panel::BackgroundColor,
				&UI::Panel::BackgroundColor,
				UI::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_SECONDARY },
				UI::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.7f
			));
		});

		return textBox;
	}

	virtual ~LoginPage() = default;

	virtual void InitializeComponents() override {
		AssetManager * assets = Service::Get<AssetManager>();

		textFont = assets->ImportFont(L"titillium18.spritefont");

		aenami = assets->ImportTexture2D(L"aenami_dreamer.jpg");
		Netcode::ResourceViewsRef aenamiRv = assets->CreateTextureRV(aenami);
		Netcode::UInt2 aenamiSize = Netcode::UInt2{ static_cast<uint32_t>(aenami->GetDesc().width), aenami->GetDesc().height };

		loadingIcon = assets->ImportTexture2D(L"loading_icon.png");
		Netcode::ResourceViewsRef loadingIconRv = assets->CreateTextureRV(loadingIcon);
		Netcode::UInt2 loadingIconSize = Netcode::UInt2{ static_cast<uint32_t>(loadingIcon->GetDesc().width), loadingIcon->GetDesc().height };

		std::shared_ptr<UI::Panel> rootPanel = std::make_shared<UI::Panel>(eventAllocator, CreatePhysxActor());
		rootPanel->BackgroundColor(Netcode::Float4::One);
		rootPanel->Sizing(UI::SizingType::INHERITED);
		rootPanel->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);
		rootPanel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		rootPanel->BackgroundImage(aenamiRv, aenamiSize);
		rootPanel->BackgroundSize(Netcode::Float2{ 1920.0f, 1080.0f });
		rootPanel->BackgroundHorizontalAlignment(UI::HorizontalAnchor::CENTER);
		rootPanel->BackgroundVerticalAlignment(UI::VerticalAnchor::MIDDLE);

		std::shared_ptr<UI::Panel> loadingIconPanel = std::make_shared<UI::Panel>(eventAllocator, CreatePhysxActor());
		loadingIconPanel->BackgroundImage(loadingIconRv, loadingIconSize);
		loadingIconPanel->Sizing(UI::SizingType::FIXED);
		loadingIconPanel->BackgroundColor(Netcode::Float4::One);
		loadingIconPanel->RotationOrigin(UI::HorizontalAnchor::CENTER, UI::VerticalAnchor::MIDDLE);
		loadingIconPanel->Size(Netcode::Float2{ 64.0f, 64.0f });

		std::shared_ptr<UI::InputGroup> inputGroup = std::make_shared<UI::InputGroup>(eventAllocator, CreatePhysxActor());
		inputGroup->Sizing(UI::SizingType::DERIVED);

		std::shared_ptr<UI::StackPanel> inputWrapper = std::make_shared<UI::StackPanel>(eventAllocator, CreatePhysxActor());
		inputWrapper->Sizing(UI::SizingType::DERIVED);
		inputWrapper->StackDirection(UI::Direction::VERTICAL);
		inputWrapper->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);

		std::shared_ptr<UI::Label> titleLabel = std::make_shared<UI::Label>(eventAllocator, CreatePhysxActor());
		titleLabel->Sizing(UI::SizingType::FIXED);
		titleLabel->Size(Netcode::Float2{ 400.0f, 100.0f });
		titleLabel->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);
		titleLabel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		titleLabel->TextColor(COLOR_ACCENT);
		titleLabel->Font(assets->ImportFont(L"titillium48bold.spritefont"));
		titleLabel->Text(L"Netcode");
		titleLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 64.0f });

		std::shared_ptr<UI::StackPanel> usernameField = std::make_shared<UI::StackPanel>(eventAllocator, CreatePhysxActor());
		usernameField->StackDirection(UI::Direction::HORIZONTAL);
		usernameField->Sizing(UI::SizingType::DERIVED);
		usernameField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

		std::shared_ptr<UI::Label> usernameLabel = std::make_shared<UI::Label>(eventAllocator, CreatePhysxActor());
		usernameLabel->Sizing(UI::SizingType::FIXED);
		usernameLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
		usernameLabel->HorizontalContentAlignment(UI::HorizontalAnchor::RIGHT);
		usernameLabel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		usernameLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });
		usernameLabel->TextColor(COLOR_ACCENT);
		usernameLabel->Font(assets->ImportFont(L"titillium18.spritefont"));
		usernameLabel->Text(L"Username:");

		std::shared_ptr<UI::TextBox> usernameTextBox = CreateTextBox();

		std::shared_ptr<UI::StackPanel> passwordField = std::make_shared<UI::StackPanel>(eventAllocator, CreatePhysxActor());
		passwordField->StackDirection(UI::Direction::HORIZONTAL);
		passwordField->Sizing(UI::SizingType::DERIVED);
		passwordField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

		std::shared_ptr<UI::Label> passwordLabel = std::make_shared<UI::Label>(eventAllocator, CreatePhysxActor());
		passwordLabel->Sizing(UI::SizingType::FIXED);
		passwordLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
		passwordLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });
		passwordLabel->HorizontalContentAlignment(UI::HorizontalAnchor::RIGHT);
		passwordLabel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		passwordLabel->TextColor(COLOR_ACCENT);
		passwordLabel->Font(assets->ImportFont(L"titillium18.spritefont"));
		passwordLabel->Text(L"Password:");

		std::shared_ptr<UI::StackPanel> buttonField = std::make_shared<UI::StackPanel>(eventAllocator, CreatePhysxActor());
		buttonField->StackDirection(UI::Direction::HORIZONTAL);
		buttonField->Sizing(UI::SizingType::DERIVED);
		buttonField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

		std::shared_ptr<UI::Button> loginButton = CreateButton(L"Login");
		std::shared_ptr<UI::Button> exitButton = CreateButton(L"Exit");
		exitButton->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });

		std::shared_ptr<UI::TextBox> passwordTextBox = CreateTextBox();
		passwordTextBox->IsPassword(true);

		passwordField->AddChild(passwordLabel);
		passwordField->AddChild(passwordTextBox);

		usernameField->AddChild(usernameLabel);
		usernameField->AddChild(usernameTextBox);

		buttonField->AddChild(exitButton);
		buttonField->AddChild(loginButton);

		inputWrapper->AddChild(titleLabel);
		inputWrapper->AddChild(usernameField);
		inputWrapper->AddChild(passwordField);
		inputWrapper->AddChild(buttonField);


		inputGroup->AddChild(inputWrapper);
		rootPanel->AddChild(inputGroup);

		//rootPanel->AddChild(loadingIconPanel);

		this->AddChild(rootPanel);
		/*
		std::unique_ptr<UI::Animation> loadingAnim = UI::MakeAnimation(
			static_cast<UI::Panel*>(rootPanel.get()),
			&UI::Panel::Opacity,
			&UI::Panel::Opacity,
			UI::Interpolator<float>{ 0.0f, 1.0f },
			UI::RepeatBehaviour{},
			&Netcode::Function::HalfStep, 1.0f
		);

		rootPanel->AddAnimation(std::move(loadingAnim));*/

		std::unique_ptr<UI::Animation> positionAnim = UI::MakeAnimation(
			static_cast<UI::Control*>(inputGroup.get()),
			&UI::Control::Position,
			&UI::Control::Position,
			UI::Interpolator<Netcode::Vector2>{ Netcode::Float2{ -1000.0f, 0.0f }, Netcode::Float2{ 0.0f, 0.0f } },
			UI::PlayOnceBehaviour{ },
			&Netcode::Function::EaseOutQuad, 0.7f);

		inputGroup->AddAnimation(std::move(positionAnim));

		UpdateZIndices();
	}


};


