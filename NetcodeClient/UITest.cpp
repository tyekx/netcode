#include "UITest.h"
#include <Netcode/UI/TextBox.h>
#include <Netcode/UI/Button.h>
#include <Netcode/UI/StackPanel.h>
#include <Netcode/UI/InputGroup.h>

namespace ui = Netcode::UI;


std::shared_ptr<ui::Button> LoginPage::CreateButton(const wchar_t * text) {
	std::shared_ptr<ui::Button> button = std::make_shared<ui::Button>(eventAllocator, CreatePhysxActor());
	button->Sizing(ui::SizingType::FIXED);
	button->Size(Netcode::Float2{ 156.0f, 48.0f });
	button->BackgroundColor(COLOR_SECONDARY);
	button->BorderColor(COLOR_HOVER);
	button->BorderRadius(BORDER_RADIUS);
	button->BorderWidth(BORDER_WIDTH);
	button->Font(textFont);
	button->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	button->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	button->Text(text);
	button->TextColor(COLOR_ACCENT);
	return button;
}

std::shared_ptr<Netcode::UI::TextBox> LoginPage::CreateTextBox() {
	std::shared_ptr<ui::TextBox> textBox = std::make_shared<ui::TextBox>(eventAllocator, CreatePhysxActor());
	textBox->Sizing(ui::SizingType::FIXED);
	textBox->Size(Netcode::Float2{ 280.0f, 48.0f });
	textBox->BackgroundColor(COLOR_SECONDARY);
	textBox->BorderColor(COLOR_HOVER);
	textBox->BorderRadius(BORDER_RADIUS);
	textBox->BorderWidth(BORDER_WIDTH);
	textBox->Font(textFont);
	textBox->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	textBox->HorizontalContentAlignment(ui::HorizontalAnchor::LEFT);
	textBox->Padding(Netcode::Float4{ 10.0f, 0.0f, 10.0f, 0.0f });
	textBox->Text(L"");
	textBox->TextColor(COLOR_ACCENT);
	textBox->SelectionBackgroundColor(COLOR_SECONDARY);
	textBox->SelectionTextColor(COLOR_ACCENT);
	textBox->CaretColor(COLOR_SECONDARY);

	textBox->OnKeyPressed.Subscribe([](ui::Control * ctrl, ui::KeyEventArgs & args) -> void {
		ui::TextBox * pThis = static_cast<ui::TextBox *>(ctrl);

		Netcode::Key key = args.Key();

		pThis->HandleDefaultKeyStrokes(key, args.Modifier());
	});

	textBox->OnCharInput.Subscribe([](ui::Control * ctrl, ui::CharInputEventArgs & args) -> void {
		wchar_t v = args.Value();
		ui::TextBox * pThis = static_cast<ui::TextBox *>(ctrl);

		if(v > 0x20) {
			pThis->AppendChar(v);
		}
	});

	textBox->OnBlurred.Subscribe([](ui::Control * ctrl, ui::FocusChangedEventArgs & args) -> void {
		auto * pThis = static_cast<ui::Input *>(ctrl);
		Netcode::Float4 targetColor = (pThis->Hovered()) ? COLOR_HOVER : COLOR_SECONDARY;
		pThis->ClearAnimations();
		pThis->AddAnimation(ui::MakeAnimation(
			static_cast<ui::Panel *>(pThis),
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), targetColor },
			ui::PlayOnceBehaviour{},
			& Netcode::Function::EaseOutQuad,
			0.7f
		));

		pThis->AddAnimation(ui::MakeAnimation(
			static_cast<ui::Label *>(pThis),
			&ui::Label::TextColor,
			&ui::Label::TextColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
			ui::PlayOnceBehaviour{},
			& Netcode::Function::EaseOutQuad,
			0.7f
		));
	});

	textBox->OnFocused.Subscribe([](ui::Control * ctrl, ui::FocusChangedEventArgs & args) -> void {
		auto * pThis = static_cast<ui::Input *>(ctrl);
		pThis->ClearAnimations();
		pThis->AddAnimation(ui::MakeAnimation(
			static_cast<ui::Panel *>(pThis),
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_ACCENT },
			ui::PlayOnceBehaviour{},
			& Netcode::Function::EaseOutQuad,
			0.7f
		));

		pThis->AddAnimation(ui::MakeAnimation(
			static_cast<ui::Label*>(pThis),
			&ui::Label::TextColor,
			&ui::Label::TextColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_SECONDARY },
			ui::PlayOnceBehaviour{},
			&Netcode::Function::EaseOutQuad,
			0.7f
		));
	});

	textBox->OnMouseEnter.Subscribe([](ui::Control * controlPointer, ui::MouseEventArgs & args) -> void {
		auto * pThis = static_cast<ui::Input *>(controlPointer);

		if(!pThis->Focused()) {
			pThis->ClearAnimations();
			pThis->AddAnimation(ui::MakeAnimation(
				static_cast<ui::Panel *>(pThis),
				&ui::Panel::BackgroundColor,
				&ui::Panel::BackgroundColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_HOVER },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.7f
			));
		}
	});

	textBox->OnMouseLeave.Subscribe([](ui::Control * controlPointer, ui::MouseEventArgs & args) -> void {
		auto * pThis = static_cast<ui::Input *>(controlPointer);

		if(!pThis->Focused()) {
			pThis->ClearAnimations();
			pThis->AddAnimation(ui::MakeAnimation(
				static_cast<ui::Panel *>(pThis),
				&ui::Panel::BackgroundColor,
				&ui::Panel::BackgroundColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_SECONDARY },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.7f
			));
		}
	});

	return textBox;
}

void LoginPage::InitializeComponents() {
	AssetManager * assets = Service::Get<AssetManager>();

	textFont = assets->ImportFont(L"titillium18.spritefont");
	/*
	aenami = assets->ImportTexture2D(L"aenami_dreamer.jpg");
	Netcode::ResourceViewsRef aenamiRv = assets->CreateTextureRV(aenami);
	Netcode::UInt2 aenamiSize = Netcode::UInt2{ static_cast<uint32_t>(aenami->GetDesc().width), aenami->GetDesc().height };

	loadingIcon = assets->ImportTexture2D(L"loading_icon.png");
	Netcode::ResourceViewsRef loadingIconRv = assets->CreateTextureRV(loadingIcon);
	Netcode::UInt2 loadingIconSize = Netcode::UInt2{ static_cast<uint32_t>(loadingIcon->GetDesc().width), loadingIcon->GetDesc().height };
	*/
	std::shared_ptr<ui::Panel> rootPanel = std::make_shared<ui::Panel>(eventAllocator, CreatePhysxActor());
	rootPanel->BackgroundColor(Netcode::Float4::One);
	rootPanel->Sizing(ui::SizingType::INHERITED);
	rootPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rootPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	//rootPanel->BackgroundImage(aenamiRv, aenamiSize);
	rootPanel->BackgroundSize(Netcode::Float2{ 1920.0f, 1080.0f });
	rootPanel->BackgroundHorizontalAlignment(ui::HorizontalAnchor::CENTER);
	rootPanel->BackgroundVerticalAlignment(ui::VerticalAnchor::MIDDLE);

	std::shared_ptr<ui::Panel> loadingIconPanel = std::make_shared<ui::Panel>(eventAllocator, CreatePhysxActor());
	//loadingIconPanel->BackgroundImage(loadingIconRv, loadingIconSize);
	loadingIconPanel->Sizing(ui::SizingType::FIXED);
	loadingIconPanel->BackgroundColor(Netcode::Float4::One);
	loadingIconPanel->RotationOrigin(ui::HorizontalAnchor::CENTER, ui::VerticalAnchor::MIDDLE);
	loadingIconPanel->Size(Netcode::Float2{ 64.0f, 64.0f });

	std::shared_ptr<ui::InputGroup> inputGroup = std::make_shared<ui::InputGroup>(eventAllocator, CreatePhysxActor());
	inputGroup->Sizing(ui::SizingType::DERIVED);

	std::shared_ptr<ui::StackPanel> inputWrapper = std::make_shared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	inputWrapper->Sizing(ui::SizingType::DERIVED);
	inputWrapper->StackDirection(ui::Direction::VERTICAL);
	inputWrapper->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);

	std::shared_ptr<ui::Label> titleLabel = std::make_shared<ui::Label>(eventAllocator, CreatePhysxActor());
	titleLabel->Sizing(ui::SizingType::FIXED);
	titleLabel->Size(Netcode::Float2{ 400.0f, 100.0f });
	titleLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	titleLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	titleLabel->TextColor(COLOR_ACCENT);
	titleLabel->Font(assets->ImportFont(L"titillium48bold.spritefont"));
	titleLabel->Text(L"Netcode");
	titleLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 64.0f });

	std::shared_ptr<ui::StackPanel> usernameField = std::make_shared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	usernameField->StackDirection(ui::Direction::HORIZONTAL);
	usernameField->Sizing(ui::SizingType::DERIVED);
	usernameField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

	std::shared_ptr<ui::Label> usernameLabel = std::make_shared<ui::Label>(eventAllocator, CreatePhysxActor());
	usernameLabel->Sizing(ui::SizingType::FIXED);
	usernameLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
	usernameLabel->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);
	usernameLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	usernameLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });
	usernameLabel->TextColor(COLOR_ACCENT);
	usernameLabel->Font(assets->ImportFont(L"titillium18.spritefont"));
	usernameLabel->Text(L"Username:");

	std::shared_ptr<ui::TextBox> usernameTextBox = CreateTextBox();

	std::shared_ptr<ui::StackPanel> passwordField = std::make_shared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	passwordField->StackDirection(ui::Direction::HORIZONTAL);
	passwordField->Sizing(ui::SizingType::DERIVED);
	passwordField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

	std::shared_ptr<ui::Label> passwordLabel = std::make_shared<ui::Label>(eventAllocator, CreatePhysxActor());
	passwordLabel->Sizing(ui::SizingType::FIXED);
	passwordLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
	passwordLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });
	passwordLabel->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);
	passwordLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	passwordLabel->TextColor(COLOR_ACCENT);
	passwordLabel->Font(assets->ImportFont(L"titillium18.spritefont"));
	passwordLabel->Text(L"Password:");

	std::shared_ptr<ui::StackPanel> buttonField = std::make_shared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	buttonField->StackDirection(ui::Direction::HORIZONTAL);
	buttonField->Sizing(ui::SizingType::DERIVED);
	buttonField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

	std::shared_ptr<ui::Button> loginButton = CreateButton(L"Login");
	std::shared_ptr<ui::Button> exitButton = CreateButton(L"Exit");
	exitButton->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });

	std::shared_ptr<ui::TextBox> passwordTextBox = CreateTextBox();
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

	std::unique_ptr<ui::Animation> positionAnim = ui::MakeAnimation(
		static_cast<ui::Control *>(inputGroup.get()),
		&ui::Control::Position,
		&ui::Control::Position,
		ui::Interpolator<Netcode::Vector2>{ Netcode::Float2{ -1000.0f, 0.0f }, Netcode::Float2{ 0.0f, 0.0f } },
		ui::PlayOnceBehaviour{ },
		& Netcode::Function::EaseOutQuad, 0.7f);

	inputGroup->AddAnimation(std::move(positionAnim));

	UpdateZIndices();
}

