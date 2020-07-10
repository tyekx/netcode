#pragma once

#include "UI/Page.h"

#include "Services.h"

class LoginPage : public UI::Page {
public:
	using UI::Page::Page;

	virtual ~LoginPage() = default;

	virtual void InitializeComponents() override {
		AssetManager * assets = Service::Get<AssetManager>();

		std::shared_ptr<UI::Panel> rootPanel = std::make_shared<UI::Panel>();
		rootPanel->BackgroundColor(Netcode::Float4::One);
		rootPanel->Sizing(UI::SizingType::WINDOW);
		rootPanel->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);
		rootPanel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);

		std::shared_ptr<UI::InputGroup> inputGroup = std::make_shared<UI::InputGroup>();
		inputGroup->Sizing(UI::SizingType::DERIVED);

		std::shared_ptr<UI::StackPanel> inputWrapper = std::make_shared<UI::StackPanel>();
		inputWrapper->Sizing(UI::SizingType::DERIVED);
		inputWrapper->StackDirection(UI::Direction::VERTICAL);

		std::shared_ptr<UI::Label> titleLabel = std::make_shared<UI::Label>();
		titleLabel->Sizing(UI::SizingType::FIXED);
		titleLabel->Size(Netcode::Float2{ 400.0f, 100.0f });
		titleLabel->HorizontalContentAlignment(UI::HorizontalAnchor::CENTER);
		titleLabel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
		titleLabel->TextColor(Netcode::Float4{ 0.2f, 0.2f, 0.2f, 1.0f });
		titleLabel->Font(assets->ImportFont(L"titillium48bold.spritefont"));
		titleLabel->Text(L"Netcode");

		std::shared_ptr<UI::StackPanel> usernameField = std::make_shared<UI::StackPanel>();
		usernameField->StackDirection(UI::Direction::HORIZONTAL);
		usernameField->Sizing(UI::SizingType::DERIVED);

		std::shared_ptr<UI::Label> usernameLabel = std::make_shared<UI::Label>();
		usernameLabel->Sizing(UI::SizingType::FIXED);
		usernameLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
		usernameLabel->HorizontalContentAlignment(UI::HorizontalAnchor::RIGHT);
		usernameLabel->VerticalContentAlignment(UI::VerticalAnchor::MIDDLE);
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

		usernameField->AddChild(usernameLabel);
		usernameField->AddChild(usernameTextBox);

		inputWrapper->AddChild(titleLabel);
		inputWrapper->AddChild(usernameField);

		inputGroup->AddChild(inputWrapper);

		rootPanel->AddChild(inputGroup);

		this->AddChild(rootPanel);

		OnInitialized();
	}
};


