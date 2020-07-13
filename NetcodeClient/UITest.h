#pragma once

#include "UI/Page.h"

#include "Services.h"

class LoginPage : public UI::Page {
	Netcode::GpuResourceRef aenami;
	Netcode::GpuResourceRef loadingIcon;
public:
	using UI::Page::Page;

	virtual ~LoginPage() = default;

	virtual void InitializeComponents() override {
		AssetManager * assets = Service::Get<AssetManager>();

		aenami = assets->ImportTexture2D(L"aenami_dreamer.jpg");
		Netcode::ResourceViewsRef aenamiRv = assets->CreateTextureRV(aenami);
		Netcode::UInt2 aenamiSize = Netcode::UInt2{ static_cast<uint32_t>(aenami->GetDesc().width), aenami->GetDesc().height };

		loadingIcon = assets->ImportTexture2D(L"loading_icon.png");
		Netcode::ResourceViewsRef loadingIconRv = assets->CreateTextureRV(loadingIcon);
		Netcode::UInt2 loadingIconSize = Netcode::UInt2{ static_cast<uint32_t>(loadingIcon->GetDesc().width), loadingIcon->GetDesc().height };

		std::shared_ptr<UI::Panel> rootPanel = std::make_shared<UI::Panel>();
		rootPanel->BackgroundColor(Netcode::Float4::One);
		rootPanel->Sizing(UI::SizingType::WINDOW);
		rootPanel->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);
		rootPanel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		rootPanel->BackgroundImage(aenamiRv, aenamiSize);
		rootPanel->BackgroundSize(Netcode::Float2{ 1920.0f, 1080.0f });
		rootPanel->BackgroundHorizontalAlignment(UI::HorizontalAnchor::CENTER);
		rootPanel->BackgroundVerticalAlignment(UI::VerticalAnchor::MIDDLE);

		std::shared_ptr<UI::Panel> loadingIconPanel = std::make_shared<UI::Panel>();
		loadingIconPanel->BackgroundImage(loadingIconRv, loadingIconSize);
		loadingIconPanel->Sizing(UI::SizingType::FIXED);
		loadingIconPanel->BackgroundColor(Netcode::Float4::One);
		loadingIconPanel->RotationOrigin(UI::HorizontalAnchor::CENTER, UI::VerticalAnchor::MIDDLE);
		loadingIconPanel->Size(Netcode::Float2{ 64.0f, 64.0f });

		std::shared_ptr<UI::InputGroup> inputGroup = std::make_shared<UI::InputGroup>();
		inputGroup->Sizing(UI::SizingType::DERIVED);

		std::shared_ptr<UI::StackPanel> inputWrapper = std::make_shared<UI::StackPanel>();
		inputWrapper->Sizing(UI::SizingType::DERIVED);
		inputWrapper->StackDirection(UI::Direction::VERTICAL);
		inputWrapper->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);

		std::shared_ptr<UI::Label> titleLabel = std::make_shared<UI::Label>();
		titleLabel->Sizing(UI::SizingType::FIXED);
		titleLabel->Size(Netcode::Float2{ 400.0f, 100.0f });
		titleLabel->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);
		titleLabel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		titleLabel->TextColor(Netcode::Float4{ 1.0f, 0.2f, 0.2f, 1.0f });
		titleLabel->Font(assets->ImportFont(L"titillium48bold.spritefont"));
		titleLabel->Text(L"Netcode");

		std::shared_ptr<UI::StackPanel> usernameField = std::make_shared<UI::StackPanel>();
		usernameField->StackDirection(UI::Direction::HORIZONTAL);
		usernameField->Sizing(UI::SizingType::DERIVED);
		usernameField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

		std::shared_ptr<UI::Label> usernameLabel = std::make_shared<UI::Label>();
		usernameLabel->Sizing(UI::SizingType::FIXED);
		usernameLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
		usernameLabel->HorizontalContentAlignment(UI::HorizontalAnchor::RIGHT);
		usernameLabel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		usernameLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });
		usernameLabel->TextColor(Netcode::Float4{ 0.2f, 0.2f, 0.2f, 1.0f });
		usernameLabel->Font(assets->ImportFont(L"titillium18.spritefont"));
		usernameLabel->Text(L"Username:");

		std::shared_ptr<UI::TextBox> usernameTextBox = std::make_shared<UI::TextBox>();
		usernameTextBox->Sizing(UI::SizingType::FIXED);
		usernameTextBox->Size(Netcode::Float2{ 280.0f, 48.0f });
		usernameTextBox->BackgroundColor(Netcode::Float4{ 0.7f, 0.7f, 0.7f, 1.0f });
		usernameTextBox->BorderColor(Netcode::Float4{ 0.2f, 0.2f, 0.2f, 1.0f });
		usernameTextBox->BorderRadius(5.0f);
		usernameTextBox->BorderWidth(3.0f);
		usernameTextBox->Font(assets->ImportFont(L"titillium18.spritefont"));
		usernameTextBox->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		usernameTextBox->HorizontalContentAlignment(UI::HorizontalAnchor::LEFT);

		std::shared_ptr<UI::StackPanel> passwordField = std::make_shared<UI::StackPanel>();
		passwordField->StackDirection(UI::Direction::HORIZONTAL);
		passwordField->Sizing(UI::SizingType::DERIVED);
		passwordField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

		std::shared_ptr<UI::Label> passwordLabel = std::make_shared<UI::Label>();
		passwordLabel->Sizing(UI::SizingType::FIXED);
		passwordLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
		passwordLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });
		passwordLabel->HorizontalContentAlignment(UI::HorizontalAnchor::RIGHT);
		passwordLabel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		passwordLabel->TextColor(Netcode::Float4{ 0.2f, 0.2f, 0.2f, 1.0f });
		passwordLabel->Font(assets->ImportFont(L"titillium18.spritefont"));
		passwordLabel->Text(L"Password:");

		std::shared_ptr<UI::StackPanel> buttonField = std::make_shared<UI::StackPanel>();
		buttonField->StackDirection(UI::Direction::HORIZONTAL);
		buttonField->Sizing(UI::SizingType::DERIVED);
		buttonField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

		std::shared_ptr<UI::Button> loginButton = std::make_shared<UI::Button>();
		loginButton->Sizing(UI::SizingType::FIXED);
		loginButton->Size(Netcode::Float2{ 156.0f, 48.0f });
		loginButton->BackgroundColor(Netcode::Float4{ 0.7f, 0.7f, 0.7f, 1.0f });
		loginButton->BorderColor(Netcode::Float4{ 0.2f, 0.2f, 0.2f, 1.0f });
		loginButton->BorderRadius(5.0f);
		loginButton->BorderWidth(3.0f);
		loginButton->Font(assets->ImportFont(L"titillium18.spritefont"));
		loginButton->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		loginButton->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);
		loginButton->Text(L"Login");
		loginButton->TextColor(Netcode::Float4{ 0.2f, 0.2f, 0.2f, 1.0f });
		loginButton->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 0.0f });

		std::shared_ptr<UI::Button> exitButton = std::make_shared<UI::Button>();
		exitButton->Sizing(UI::SizingType::FIXED);
		exitButton->Size(Netcode::Float2{ 156.0f, 48.0f });
		exitButton->BackgroundColor(Netcode::Float4{ 0.7f, 0.7f, 0.7f, 1.0f });
		exitButton->BorderColor(Netcode::Float4{ 0.2f, 0.2f, 0.2f, 1.0f });
		exitButton->BorderRadius(5.0f);
		exitButton->BorderWidth(3.0f);
		exitButton->Font(assets->ImportFont(L"titillium18.spritefont"));
		exitButton->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		exitButton->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);
		exitButton->Text(L"Exit");
		exitButton->TextColor(Netcode::Float4{ 0.2f, 0.2f, 0.2f, 1.0f });
		exitButton->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });

		std::shared_ptr<UI::TextBox> passwordTextBox = std::make_shared<UI::TextBox>();
		passwordTextBox->Sizing(UI::SizingType::FIXED);
		passwordTextBox->Size(Netcode::Float2{ 280.0f, 48.0f });
		passwordTextBox->BackgroundColor(Netcode::Float4{ 0.7f, 0.7f, 0.7f, 1.0f });
		passwordTextBox->BorderColor(Netcode::Float4{ 0.2f, 0.2f, 0.2f, 1.0f });
		passwordTextBox->BorderRadius(5.0f);
		passwordTextBox->BorderWidth(3.0f);
		passwordTextBox->Font(assets->ImportFont(L"titillium18.spritefont"));
		passwordTextBox->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		passwordTextBox->HorizontalContentAlignment(UI::HorizontalAnchor::LEFT);

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

		std::unique_ptr<UI::Animation> loadingAnim = UI::MakeAnimation(
			static_cast<UI::Panel*>(rootPanel.get()),
			&UI::Panel::Opacity,
			&UI::Panel::Opacity,
			UI::Interpolator<float>{ 0.0f, 1.0f },
			UI::RepeatBehaviour{},
			&Netcode::Function::HalfStep, 1.0f
		);

		rootPanel->AddAnimation(std::move(loadingAnim));

		std::unique_ptr<UI::Animation> positionAnim = UI::MakeAnimation(
			static_cast<UI::Control*>(inputGroup.get()),
			&UI::Control::Position,
			&UI::Control::Position,
			UI::Interpolator<Netcode::Vector2>{ Netcode::Float2{ -1000.0f, 0.0f }, Netcode::Float2{ 0.0f, 0.0f } },
			UI::PlayOnceBehaviour{ },
			&Netcode::Function::EaseOutQuad, 0.7f);

		inputGroup->AddAnimation(std::move(positionAnim));

		OnInitialized();
	}
};


