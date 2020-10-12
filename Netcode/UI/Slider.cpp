#include "Slider.h"
#include <physx/PxPhysicsAPI.h>
#include <sstream>
#include <iomanip>

namespace Netcode::UI {

    namespace Detail {
    	
        class DragBtn : public Panel {
        public:
            using Panel::Panel;

            virtual void PropagateOnPositionChanged() override {
                screenPositionCache = CalculateScreenPosition();
                UpdateActorPose();
            }

            virtual void PropagateOnSizeChanged() override {
                UpdateActorShape();
            }

            virtual void PropagateOnDrag(DragEventArgs & args) override {
                args.Handled(true);
                args.HandledBy(this);
                Parent()->Parent()->PropagateOnDrag(args);
            }
        };
    }

    void Slider::UpdateLabelText() {
        if(sliderValue != nullptr) {
        	
            std::wostringstream woss;
            woss << std::fixed << std::setprecision(6) << value;
            sliderValue->Text(woss.str());
        }
    }

    void Slider::UpdateFromSlider(const Int2 & deltaPosition)
    {
        const float deltaX = static_cast<float>(deltaPosition.x);
        const float minX = 0.0f;
        const float maxX = sliderPanel->SizeX() - sliderButton->SizeX();
        const Float2 current = sliderButton->Position();
        const float updatedX = std::clamp(current.x + deltaX, minX, maxX);

        sliderButton->Position(Float2{ updatedX, current.y });

        const double cx = static_cast<double>(updatedX) / static_cast<double>(maxX);
        const double minV = MinValue();
        const double maxV = MaxValue();
        const double representedValue = cx * maxV + (1.0 - cx) * minV;
        value = representedValue;
        textUpdating = true;
        UpdateLabelText();
        textUpdating = false;
        OnValueChanged.Invoke(value);
    }

    void Slider::UpdateFromText(const std::wstring & strValue)
    {
    	// dirty hack to avoid feedbacks
    	if(textUpdating) {
            return;
    	}
    	
        const double convertedValue = wcstod(strValue.c_str(), nullptr);
        const float maxX = sliderPanel->SizeX() - sliderButton->SizeX();

        const double p = std::clamp(convertedValue, MinValue(), MaxValue());
        const double dist = MaxValue() - MinValue();

        value = convertedValue;
    	
    	// p / dist = x / (maxX - minX)
        const float posX = static_cast<float>(((p - MinValue()) / dist) * (static_cast<double>(maxX)));

        const Float2 pos = sliderButton->Position();
        sliderButton->Position(Float2{ posX, pos.y });

        OnValueChanged.Invoke(value);
    }

    void Slider::UpdateFromValue(double d) {
        value = d;
        const float maxX = sliderPanel->SizeX() - sliderButton->SizeX();
        const double p = std::clamp(d, MinValue(), MaxValue());
        const double dist = MaxValue() - MinValue();
        const float posX = static_cast<float>(((p - MinValue()) / dist) * (static_cast<double>(maxX)));

        const Float2 pos = sliderButton->Position();
        sliderButton->Position(Float2{ posX, pos.y });
        textUpdating = true;
        UpdateLabelText();
        textUpdating = false;
        OnValueChanged.Invoke(value);
    }

    Slider::Slider(Memory::ObjectAllocator controlAllocator, const AllocType & alloc, PxPtr<physx::PxRigidDynamic> actor) : StackPanel{ alloc, std::move(actor) },
		sliderPanel{ nullptr },
        sliderButton{ nullptr },
        sliderValue{ nullptr },
		minValue{ 0.0 },
		maxValue{ 1.0 },
		value{ 0.0 },
		onTextChangedToken{},
		textUpdating{false},
		OnValueChanged{ alloc } {
        Sizing(SizingType::DERIVED);
        StackDirection(Direction::HORIZONTAL);
        HorizontalContentAlignment(HorizontalAnchor::LEFT);
        VerticalContentAlignment(VerticalAnchor::MIDDLE);

        physx::PxScene * scene = pxActor->getScene();

        UndefinedBehaviourAssertion(scene != nullptr);
        UndefinedBehaviourAssertion(pxActor->getNbShapes() == 1);

        physx::PxShape * shape;
        pxActor->getShapes(&shape, 1);

        UndefinedBehaviourAssertion(shape->getNbMaterials() == 1);

        physx::PxMaterial * mat;
        shape->getMaterials(&mat, 1);

        PxPtr<physx::PxRigidDynamic> clonedActor = physx::PxCreateDynamic(scene->getPhysics(), physx::PxTransform{ physx::PxIdentity }, physx::PxBoxGeometry{ 1.0f, 1.0f, 1.0f }, *mat, 0.5f);
        scene->addActor(*clonedActor);

        sliderButton = controlAllocator.MakeShared<Detail::DragBtn>(alloc, std::move(clonedActor));
        sliderButton->Sizing(SizingType::FIXED);
        sliderButton->Size(Float2{ 20.0f, 40.0f });
        sliderButton->BackgroundColor(Float4::One);
        sliderButton->ZIndex(64.0f);
        sliderButton->Enabled(true);

    	sliderPanel = controlAllocator.MakeShared<Panel>(alloc, nullptr);
        sliderPanel->Sizing(SizingType::FIXED);
        sliderPanel->HorizontalContentAlignment(HorizontalAnchor::LEFT);
        sliderPanel->VerticalContentAlignment(VerticalAnchor::MIDDLE);
        sliderPanel->Size(Float2{ 196.0f, 8.0f });
        sliderPanel->Margin(Float4{ 0.0f, 0.0f, 16.0f, 0.0 });
        sliderPanel->BackgroundColor(Float4::One);
        sliderPanel->ZIndex(63.0f);
        sliderPanel->Enabled(true);
        sliderPanel->AddChild(sliderButton);
    }

    void Slider::PropagateOnDrag(DragEventArgs & args) {
        if(args.Handled()) {
            if(args.DeltaPosition().x == 0) {
                return;
            }
            UpdateFromSlider(args.DeltaPosition());
        }
    }

    void Slider::BindTextBox(Ref<TextBox> textBox) {
        if(onTextChangedToken != 0 && sliderValue != nullptr) {
            sliderValue->OnTextChanged.Erase(onTextChangedToken);
            onTextChangedToken = 0;
            sliderValue = nullptr;
        }

        textBox->ZIndex(63.0f);
        textBox->OnCharInput.Clear();
        textBox->OnCharInput.Subscribe([](Control * ctrl, CharInputEventArgs & args) -> void {
            wchar_t v = args.Value();
            TextBox * pThis = static_cast<TextBox *>(ctrl);

            if(pThis->Text().size() >= 8) {
                return;
            }

            if(v >= L'0' && v <= L'9') {
                pThis->AppendChar(v);
            }

            if(v == L'-') {
                if(pThis->Text().empty() || (pThis->Text().front() != L'-' && pThis->CaretPosition() == 0)) {
                    pThis->AppendChar(v);
                }
            }

            if(v == L'.') {
                if(pThis->Text().find_first_of(L'.') == std::wstring::npos) {
                    pThis->AppendChar(v);
                }
            }
        });

        onTextChangedToken = textBox->OnTextChanged.Subscribe([this](const std::wstring & cb) -> void {
            UpdateFromText(cb);
        });

        sliderValue = textBox;
    }

    void Slider::AddChild(Ref<Control> child) {
    	
    }

    void Slider::UpdateLayout() {
        if(children.empty()) {
            Panel::AddChild(sliderPanel);
        	if(sliderValue != nullptr) {
                Panel::AddChild(sliderValue);
                UpdateFromValue(value);
        	}
        }

        StackPanel::UpdateLayout();
    }

	

}
