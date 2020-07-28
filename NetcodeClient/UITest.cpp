#include "UITest.h"
#include <Netcode/UI/TextBox.h>
#include <Netcode/UI/Button.h>
#include <Netcode/UI/StackPanel.h>
#include <Netcode/UI/InputGroup.h>
#include <Netcode/UI/ScrollViewer.h>

namespace ui = Netcode::UI;


std::shared_ptr<ui::Button> PageBase::CreateButton(const wchar_t * text) {
	std::shared_ptr<ui::Button> button = controlAllocator.MakeShared<ui::Button>(eventAllocator, CreatePhysxActor());
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

	button->OnMouseKeyPressed.Subscribe([](ui::Control * ctrl, ui::MouseEventArgs & args) -> void {
		if(args.Key().GetCode() == Netcode::KeyCode::MOUSE_LEFT) {
			auto * pThis = static_cast<ui::Input *>(ctrl);
			pThis->ClearAnimations();
			pThis->AddAnimation(ui::MakeAnimation(
				static_cast<ui::Panel *>(pThis),
				&ui::Panel::BackgroundColor,
				&ui::Panel::BackgroundColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_ACCENT },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.15f
			));

			pThis->AddAnimation(ui::MakeAnimation(
				static_cast<ui::Label *>(pThis),
				&ui::Label::TextColor,
				&ui::Label::TextColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_SECONDARY },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.15f
			));
		}
	});

	button->OnMouseKeyReleased.Subscribe([](ui::Control * ctrl, ui::MouseEventArgs & args) -> void {
		if(args.Key().GetCode() == Netcode::KeyCode::MOUSE_LEFT) {
			auto * pThis = static_cast<ui::Input *>(ctrl);
			pThis->ClearAnimations();
			pThis->AddAnimation(ui::MakeAnimation(
				static_cast<ui::Panel *>(pThis),
				&ui::Panel::BackgroundColor,
				&ui::Panel::BackgroundColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_HOVER },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.35f
			));

			pThis->AddAnimation(ui::MakeAnimation(
				static_cast<ui::Label *>(pThis),
				&ui::Label::TextColor,
				&ui::Label::TextColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.35f
			));
		}
	});

	button->OnMouseEnter.Subscribe([](ui::Control * ctrl, ui::MouseEventArgs & args) -> void {
		ui::Label * pThis = static_cast<ui::Label *>(ctrl);

		ctrl->ClearAnimations();
		ctrl->AddAnimation(ui::MakeAnimation(
			static_cast<ui::Panel*>(ctrl),
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>(pThis->BackgroundColor(), COLOR_HOVER),
			ui::PlayOnceBehaviour{},
			&Netcode::Function::EaseOutQuad,
			0.7f
		));

		ctrl->AddAnimation(ui::MakeAnimation(
			pThis,
			&ui::Label::TextColor,
			&ui::Label::TextColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
			ui::PlayOnceBehaviour{},
			& Netcode::Function::EaseOutQuad,
			0.7f
		));
	});

	button->OnMouseLeave.Subscribe([](ui::Control * ctrl, ui::MouseEventArgs & args) -> void {
		ui::Label * pThis = static_cast<ui::Label *>(ctrl);

		ctrl->ClearAnimations();
		ctrl->AddAnimation(ui::MakeAnimation(
			static_cast<ui::Panel *>(ctrl),
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>(pThis->BackgroundColor(), COLOR_SECONDARY),
			ui::PlayOnceBehaviour{},
			&Netcode::Function::EaseOutQuad,
			0.7f
		));

		ctrl->AddAnimation(ui::MakeAnimation(
			pThis,
			&ui::Label::TextColor,
			&ui::Label::TextColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
			ui::PlayOnceBehaviour{},
			& Netcode::Function::EaseOutQuad,
			0.7f
		));
	});

	return button;
}

std::shared_ptr<Netcode::UI::TextBox> PageBase::CreateTextBox() {
	std::shared_ptr<ui::TextBox> textBox = controlAllocator.MakeShared<ui::TextBox>(eventAllocator, CreatePhysxActor());
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

			pThis->AddAnimation(ui::MakeAnimation(
				static_cast<ui::Label *>(pThis),
				&ui::Label::TextColor,
				&ui::Label::TextColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
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

			pThis->AddAnimation(ui::MakeAnimation(
				static_cast<ui::Label *>(pThis),
				&ui::Label::TextColor,
				&ui::Label::TextColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.7f
			));
		}
	});

	return textBox;
}

static void LogControls(int depth, const std::vector<std::shared_ptr<ui::Control>> & children) {
	std::string prefix(depth, '\t');

	for(const auto & i : children) {
		Netcode::Float2 sp = i->ScreenPosition();
		Netcode::Float2 sz = i->Size();

		std::ostringstream oss;
		oss << prefix << "WindowCoords: (" << sp.x << "; " << sp.y << ") Size: (" << sz.x << "; " << sz.y << ")";
		
		Log::Debug(oss.str().c_str());
		LogControls(depth + 1, i->Children());
	}
}

void PageBase::DebugDump()
{
	LogControls(0, Children());
}

void PageBase::InitializeComponents() {
	AssetManager * assets = Service::Get<AssetManager>();

	textFont = assets->ImportFont(L"titillium18.spritefont");
}

void LoginPage::InitializeComponents() {
	PageBase::InitializeComponents();

	AssetManager * assets = Service::Get<AssetManager>();
	/*
	aenami = assets->ImportTexture2D(L"aenami_dreamer.jpg");
	Netcode::ResourceViewsRef aenamiRv = assets->CreateTextureRV(aenami);
	Netcode::UInt2 aenamiSize = Netcode::UInt2{ static_cast<uint32_t>(aenami->GetDesc().width), aenami->GetDesc().height };
	*/
	std::shared_ptr<ui::Panel> rootPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	rootPanel->BackgroundColor(Netcode::Float4::One);
	rootPanel->Sizing(ui::SizingType::INHERITED);
	rootPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rootPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	//rootPanel->BackgroundImage(aenamiRv, aenamiSize);
	//rootPanel->BackgroundSize(Netcode::Float2{ 1920.0f, 1080.0f });
	rootPanel->BackgroundHorizontalAlignment(ui::HorizontalAnchor::CENTER);
	rootPanel->BackgroundVerticalAlignment(ui::VerticalAnchor::MIDDLE);


	std::shared_ptr<ui::InputGroup> inputGroup = controlAllocator.MakeShared<ui::InputGroup>(eventAllocator, CreatePhysxActor());
	inputGroup->Sizing(ui::SizingType::DERIVED);

	std::shared_ptr<ui::StackPanel> inputWrapper = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	inputWrapper->Sizing(ui::SizingType::DERIVED);
	inputWrapper->StackDirection(ui::Direction::VERTICAL);
	inputWrapper->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);

	std::shared_ptr<ui::Label> titleLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	titleLabel->Sizing(ui::SizingType::FIXED);
	titleLabel->Size(Netcode::Float2{ 400.0f, 100.0f });
	titleLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	titleLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	titleLabel->TextColor(COLOR_ACCENT);
	titleLabel->Font(assets->ImportFont(L"titillium48bold.spritefont"));
	titleLabel->Text(L"Netcode");
	titleLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 64.0f });

	std::shared_ptr<ui::StackPanel> usernameField = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	usernameField->StackDirection(ui::Direction::HORIZONTAL);
	usernameField->Sizing(ui::SizingType::DERIVED);
	usernameField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

	std::shared_ptr<ui::Label> usernameLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	usernameLabel->Sizing(ui::SizingType::FIXED);
	usernameLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
	usernameLabel->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);
	usernameLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	usernameLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });
	usernameLabel->TextColor(COLOR_ACCENT);
	usernameLabel->Font(assets->ImportFont(L"titillium18.spritefont"));
	usernameLabel->Text(L"Username:");

	std::shared_ptr<ui::TextBox> usernameTextBox = CreateTextBox();

	std::shared_ptr<ui::StackPanel> passwordField = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	passwordField->StackDirection(ui::Direction::HORIZONTAL);
	passwordField->Sizing(ui::SizingType::DERIVED);
	passwordField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

	std::shared_ptr<ui::Label> passwordLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	passwordLabel->Sizing(ui::SizingType::FIXED);
	passwordLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
	passwordLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });
	passwordLabel->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);
	passwordLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	passwordLabel->TextColor(COLOR_ACCENT);
	passwordLabel->Font(assets->ImportFont(L"titillium18.spritefont"));
	passwordLabel->Text(L"Password:");

	std::shared_ptr<ui::StackPanel> buttonField = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	buttonField->StackDirection(ui::Direction::HORIZONTAL);
	buttonField->Sizing(ui::SizingType::DERIVED);
	buttonField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

	std::shared_ptr<ui::Button> loginButton = CreateButton(L"Login");
	loginButton->Margin(Netcode::Float4{ 0.0f, 0.0f, 20.0f, 0.0f });
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

	this->AddChild(rootPanel);

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

std::shared_ptr<ui::Control> ServerBrowserPage::CreateServerRow(std::wstring serverIp)
{
	std::shared_ptr<ui::Label> label = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	label->Sizing(ui::SizingType::FIXED);
	label->Size(Netcode::Float2{ 480.0f, 48.0f });
	label->BackgroundColor(COLOR_SECONDARY);
	label->HorizontalContentAlignment(ui::HorizontalAnchor::LEFT);
	label->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	label->TextColor(COLOR_ACCENT);
	label->Font(textFont);
	label->Text(std::move(serverIp));

	return label;
}

void ServerBrowserPage::InitializeComponents()
{
	PageBase::InitializeComponents();

	std::shared_ptr<ui::Panel> rootPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	rootPanel->BackgroundColor(Netcode::Float4::One);
	rootPanel->Sizing(ui::SizingType::INHERITED);
	rootPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rootPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);

	std::shared_ptr<ui::ScrollViewer> scrollViewer = controlAllocator.MakeShared<ui::ScrollViewer>(controlAllocator, eventAllocator, CreatePhysxActor());
	scrollViewer->Sizing(ui::SizingType::DERIVED);
	scrollViewer->MaxSize(Netcode::Float2{ 480.0f, 640.0f });
	scrollViewer->ScrollBarColor(COLOR_HOVER);
	scrollViewer->ScrollButtonColor(COLOR_ACCENT);
	scrollViewer->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);

	std::shared_ptr<ui::StackPanel> serversListPanel = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	serversListPanel->Sizing(ui::SizingType::DERIVED);

	
	for(uint32_t i = 0; i < 32; ++i) {
		std::wstring txt = L"192.168.0." + std::to_wstring(i);
		
		serversListPanel->AddChild(CreateServerRow(std::move(txt)));
	}

	scrollViewer->AddChild(serversListPanel);

	rootPanel->AddChild(scrollViewer);

	this->AddChild(rootPanel);

	UpdateZIndices();
}

void LoadingPage::CloseDialog()
{
	rootPanel->AddAnimation(ui::MakeAnimation(
		rootPanel.get(),
		&ui::Control::SizeY,
		&ui::Control::SizeY,
		ui::Interpolator<float>{ rootPanel->SizeY(), 0.0f },
		ui::PlayOnceBehaviour{  },
		& Netcode::Function::EaseOutQuad,
		0.25f
	));

	rootPanel->OnAnimationsFinished.Subscribe([this](ui::Control *) -> void {
		Enabled(false);
		Deactivate();
	});
}

void LoadingPage::InitializeComponents() {
	PageBase::InitializeComponents();

	HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);

	AssetManager * assets = Service::Get<AssetManager>();

	loadingIcon = assets->ImportTexture2D(L"loading_icon.png");
	Netcode::ResourceViewsRef loadingIconRv = assets->CreateTextureRV(loadingIcon);
	Netcode::UInt2 loadingIconSize = Netcode::UInt2{ static_cast<uint32_t>(loadingIcon->GetDesc().width), loadingIcon->GetDesc().height };

	warningIcon = assets->ImportTexture2D(L"warning_icon.png");
	Netcode::ResourceViewsRef warningIconRv = assets->CreateTextureRV(warningIcon);
	Netcode::UInt2 warningIconSize = Netcode::UInt2{ static_cast<uint32_t>(warningIcon->GetDesc().width), warningIcon->GetDesc().height };

	std::shared_ptr<ui::Panel> rPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	rPanel->BackgroundColor(COLOR_SECONDARY);
	rPanel->Sizing(ui::SizingType::FIXED);
	rPanel->Size(Netcode::Float2{ 1920.0f, 300.0f });
	rPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	rPanel->Overflow(ui::OverflowType::HIDDEN);
	
	std::shared_ptr<ui::StackPanel> contentPanel = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	contentPanel->Sizing(ui::SizingType::DERIVED);
	contentPanel->StackDirection(ui::Direction::VERTICAL);
	contentPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);

	std::shared_ptr<ui::Panel> warningIconPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	warningIconPanel->BackgroundImage(warningIconRv, warningIconSize);
	warningIconPanel->Sizing(ui::SizingType::FIXED);
	warningIconPanel->Size(Netcode::Float2{ 80.0f, 80.0f });
	warningIconPanel->BackgroundColor(COLOR_ACCENT);

	std::shared_ptr<ui::Panel> loadingIconPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	loadingIconPanel->BackgroundImage(loadingIconRv, loadingIconSize);
	loadingIconPanel->Sizing(ui::SizingType::FIXED);
	loadingIconPanel->Size(Netcode::Float2{ 80.0f, 80.0f });
	loadingIconPanel->BackgroundColor(COLOR_ACCENT);
	loadingIconPanel->RotationOrigin(ui::HorizontalAnchor::CENTER, ui::VerticalAnchor::MIDDLE);

	std::shared_ptr<ui::Button> ackBtn = CreateButton(L"Aight");

	std::shared_ptr<ui::Label> loadingLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	loadingLabel->Text(L"You broke it");
	loadingLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	loadingLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	loadingLabel->TextColor(COLOR_ACCENT);
	loadingLabel->Sizing(ui::SizingType::FIXED);
	loadingLabel->Size(Netcode::Float2{ 200.0f, 40.0f });
	loadingLabel->Font(textFont);

	std::unique_ptr<ui::Animation> loadingAnim = ui::MakeAnimation(
		static_cast<ui::Control *>(loadingIconPanel.get()),
		&ui::Control::RotationZ,
		&ui::Control::RotationZ,
		ui::Interpolator<float>{ 0.0f, Netcode::C_2PI },
		ui::RepeatBehaviour{},
		&Netcode::Function::LerpIn, 0.7f
	);

	ackBtn->OnClick.Subscribe([this](ui::Control *, ui::MouseEventArgs &) ->void {
		CloseDialog();
	});

	loadingIconPanel->AddAnimation(std::move(loadingAnim));

	//contentPanel->AddChild(loadingIconPanel);
	contentPanel->AddChild(warningIconPanel);
	contentPanel->AddChild(loadingLabel);
	contentPanel->AddChild(ackBtn);
	rPanel->AddChild(contentPanel);
	this->AddChild(rPanel);

	rootPanel = rPanel;

	UpdateZIndices();
}

void LoadingPage::Activate() {
	Page::Activate();

	static_cast<ui::Panel *>(rootPanel.get())->BackgroundColor(Netcode::Float4::One);
	rootPanel->Size(Netcode::Float2{ 0.0f, 20.0f });

	rootPanel->AddAnimation(ui::MakeAnimation(
		rootPanel.get(),
		&ui::Control::SizeX,
		&ui::Control::SizeX,
		ui::Interpolator<float>{ 0.0f, 1920.0f },
		ui::PlayOnceBehaviour{ 1.0f },
		&Netcode::Function::EaseOutQuad,
		0.5f
	));

	rootPanel->AddAnimation(ui::MakeAnimation(
		static_cast<ui::Panel*>(rootPanel.get()),
		&ui::Panel::BackgroundColor,
		&ui::Panel::BackgroundColor,
		ui::Interpolator<Netcode::Vector4>{ COLOR_ACCENT, COLOR_SECONDARY },
		ui::PlayOnceBehaviour{ 1.25f },
		& Netcode::Function::EaseOutQuad,
		0.35f
	));

	rootPanel->AddAnimation(ui::MakeAnimation(
		rootPanel.get(),
		&ui::Control::SizeY,
		&ui::Control::SizeY,
		ui::Interpolator<float>{ 20.0f, 300.0f },
		ui::PlayOnceBehaviour{ 1.25f },
		&Netcode::Function::EaseOutQuad,
		0.5f
	));
}

void LoadingPage::SetError(const std::wstring & msg) {
	errorContent->Enabled(true);
}
