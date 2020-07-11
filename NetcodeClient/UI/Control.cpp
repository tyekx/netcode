#include "Control.h"

namespace UI {


    Control::Control() : std::enable_shared_from_this<Control>{}, size{ Netcode::Float2::Zero }, position{ Netcode::Float2::Zero }, anchorOffset{ Netcode::Float2::Zero },
        margin{ Netcode::Float4::Zero }, padding{ Netcode::Float4::Zero }, rotationZ{ 0.0f }, sizing{ SizingType::FIXED }, horizontalContentAlignment{ HorizontalAnchor::LEFT },
        verticalContentAlignment{ VerticalAnchor::TOP }, parent{ nullptr }, children{}, enabled{ true } { }

    void Control::AnchorOffset(const Netcode::Float2 & layoutPos)
    {
        anchorOffset = layoutPos;

        OnLayoutChanged();
    }

    Netcode::Float2 Control::CalculateAnchorOffset(HorizontalAnchor xAnchor, VerticalAnchor yAnchor, const Netcode::Float2 & controlSize) {
        Netcode::Float2 anchorOffset;

        switch(xAnchor) {
            case HorizontalAnchor::LEFT:
                anchorOffset.x = 0.0f;
                break;
            case HorizontalAnchor::CENTER:
                anchorOffset.x = controlSize.x / 2.0f;
                break;
            case HorizontalAnchor::RIGHT:
                anchorOffset.x = controlSize.x;
                break;
        }

        switch(yAnchor) {
            case VerticalAnchor::TOP:
                anchorOffset.y = 0.0f;
                break;
            case VerticalAnchor::MIDDLE:
                anchorOffset.y = controlSize.y / 2.0f;
                break;
            case VerticalAnchor::BOTTOM:
                anchorOffset.y = controlSize.y;
                break;
        }

        return anchorOffset;
    }

    HorizontalAnchor Control::HorizontalContentAlignment() const {
        return horizontalContentAlignment;
    }

    void Control::HorizontalContentAlignment(HorizontalAnchor horizontalAnchor) {
        horizontalContentAlignment = horizontalAnchor;
    }

    VerticalAnchor Control::VerticalContentAlignment() const {
        return verticalContentAlignment;
    }

    void Control::VerticalContentAlignment(VerticalAnchor verticalAnchor) {
        verticalContentAlignment = verticalAnchor;
    }

    SizingType Control::Sizing() const {
        return sizing;
    }

    void Control::Sizing(SizingType sizingType) {
        sizing = sizingType;
    }

    Netcode::Float2 Control::Size() const {
        return size;
    }

    void Control::Size(const Netcode::Float2 & sz) {
         size = sz;
    }

    Netcode::Float2 Control::BoxSize() const
    {
        Netcode::Float2 cSize = CalculatedSize();
        Netcode::Float4 m = Margin();

        return Netcode::Float2{ cSize.x + m.x + m.z, cSize.y + m.y + m.w };
    }

    Netcode::Float2 Control::CalculatedSize() const
    {
        Netcode::Vector2 s = Size();
        Netcode::Float4 p = Padding();

        Netcode::Vector2 horizontal = Netcode::Float2{ p.x + p.z, 0.0f };
        Netcode::Vector2 vertical = Netcode::Float2{ 0.0f, p.y + p.w };

        return s + horizontal + vertical;
    }

    Netcode::Float2 Control::Position() const {
        return position;
    }

    void Control::Position(const Netcode::Float2 & pos) {
        position = pos;
    }

    Netcode::Float2 Control::ScreenPosition() const
    {
        Netcode::Vector2 parentScreenPos = Netcode::Float2::Zero;
        Netcode::Vector2 anchorOffs = Netcode::Float2::Zero;
        Netcode::Vector2 anchorDiff = Netcode::Float2::Zero;

        if(parent != nullptr) {
            parentScreenPos = parent->ScreenPosition();
            anchorOffs = parent->AnchorOffset();
            anchorDiff = CalculateAnchorOffset(parent->HorizontalContentAlignment(), parent->VerticalContentAlignment(), BoxSize());
        }

        return parentScreenPos + anchorOffs - anchorDiff + Position();
    }

    Netcode::Float2 Control::AnchorOffset() const
    {
        return anchorOffset;
    }

    Netcode::Float4 Control::Margin() const {
        return margin;
    }

    void Control::Margin(const Netcode::Float4 & leftTopRightBottom) {
        margin = leftTopRightBottom;
    }

    Netcode::Float4 Control::Padding() const {
        return padding;
    }

    void Control::Padding(const Netcode::Float4 & leftTopRightBottom) {
        padding = leftTopRightBottom;
    }

    void Control::UpdateSize()
    {
        for(auto & child : children) {
            child->UpdateSize();
        }

        if(Sizing() == SizingType::INHERITED) {
            auto parentControl = Parent();
            if(parentControl != nullptr) {
                Size(parentControl->Size());
            } else {
                Size(Netcode::Float2::Zero);
            }
        }

        if(Sizing() == SizingType::DERIVED) {
            Netcode::Float2 maxSize;
            for(auto & child : children) {
                const Netcode::Float2 size = child->Size();

                if(maxSize.x < size.x) {
                    maxSize.x = size.x;
                }

                if(maxSize.y < size.y) {
                    maxSize.y = size.y;
                }
            }
            
            Size(maxSize);
        }
    }

    void Control::UpdateLayout()
    {
        AnchorOffset(CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), Size()));

        for(auto & child : children) {
            child->UpdateLayout();
        }
    }

    void Control::Destruct() {
        Parent(nullptr);

        for(auto & child : children) {
            child->Destruct();
        }

        children.clear();
    }

    void Control::Enable() {
        if(!enabled) {
            enabled = true;
            OnEnabled();
        }
    }

    void Control::Disable() {
        if(enabled) {
            enabled = false;
            OnDisabled();
        }
    }

    void Control::Parent(std::shared_ptr<Control> newParent) {
        if(parent != newParent) {
            parent = newParent;
        }
    }

    std::shared_ptr<Control> Control::Parent() const {
        return parent;
    }

    const std::vector<std::shared_ptr<Control>> & Control::Children() const {
        return children;
    }

    void Control::RotationZ(float rotZ) {
        rotationZ = rotZ;
    }

    float Control::RotationZ() const {
        return rotationZ;
    }

    void Control::AddChild(std::shared_ptr<Control> child) {
        children.push_back(child);
        child->Parent(shared_from_this());
    }

    void Control::OnScreenResized(const Netcode::UInt2 & newSize)
    {
        for(auto & child : children) {
            child->OnScreenResized(newSize);
        }

        if(Sizing() == SizingType::DERIVED) {
            UpdateSize();
        }

        if(Sizing() == SizingType::WINDOW) {
            Size(Netcode::Float2{ static_cast<float>(newSize.x), static_cast<float>(newSize.y) });
            UpdateLayout();
        }
    }

    void Control::OnFocused(FocusChangedEventArgs & args) {
        if(args.Handled()) {
            return;
        }

        for(auto & child : Children()) {
            child->OnFocused(args);

            if(args.Handled()) {
                return;
            }
        }
    }

    void Control::OnInitialized() {

    }

    void Control::OnLayoutChanged() {

    }

    void Control::OnKeyDown(KeyboardEventArgs & args) {

    }

    void Control::OnKeyUp(KeyboardEventArgs & args) {

    }

    void Control::OnMouseEnter(MouseEventArgs & args) {
        for(auto & child : children) {
            if(args.Handled()) {
                return;
            }
            child->OnMouseEnter(args);
        }
    }

    void Control::OnMouseLeave(MouseEventArgs & args) {
        for(auto & child : children) {
            if(args.Handled()) {
                return;
            }
            child->OnMouseEnter(args);
        }
    }

    void Control::OnRender(Netcode::SpriteBatchRef batch) {
        for(auto & child : children) {
            child->OnRender(batch);
        }
    }

    void Control::OnEnabled() {
        if(!enabled) {
            return;
        }

        for(auto & child : children) {
            child->OnEnabled();
        }
    }

    void Control::OnDisabled() {
        for(auto & child : children) {
            child->OnDisabled();
        }
    }

    void Control::OnSizeChanged() {

    }

    bool Control::IsFocused() {
        return false;
    }

    bool Control::IsEnabled() {
        return enabled;
    }

    void Control::OnClick(MouseEventArgs & args) {

    }

    void Label::Font(Netcode::SpriteFontRef ref) {
        if(font != ref) {
            font = ref;
            OnFontChanged();
        }
    }

    Netcode::SpriteFontRef Label::Font() const {
        return font;
    }

}
