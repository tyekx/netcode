#include "Control.h"

namespace UI {


    Control::Control() : std::enable_shared_from_this<Control>{},
        size{ Netcode::Float2::Zero },
        position{ Netcode::Float2::Zero },
        rotationOrigin{ Netcode::Float2::Zero },
        margin{ Netcode::Float4::Zero },
        padding{ Netcode::Float4::Zero },
        rotationZ{ 0.0f },
        zIndex{ 0.0f },
        zIndexSizing{ SizingType::DERIVED },
        rotationOriginSizing{ SizingType::FIXED },
        sizing{ SizingType::FIXED },
        horizontalRotationOrigin{ HorizontalAnchor::LEFT },
        verticalRotationOrigin{ VerticalAnchor::TOP },
        horizontalContentAlignment{ HorizontalAnchor::LEFT },
        verticalContentAlignment{ VerticalAnchor::TOP },
        parent{ nullptr },
        children{},
        animations{},
        enabled{ true } { }

    HorizontalAnchor Control::HorizontalRotationOrigin() const {
        return horizontalRotationOrigin;
    }

    void Control::HorizontalRotationOrigin(HorizontalAnchor xAnchor) {
        horizontalRotationOrigin = xAnchor;
    }

    VerticalAnchor Control::VerticalRotationOrigin() const {
        return verticalRotationOrigin;
    }

    void Control::VerticalRotationOrigin(VerticalAnchor yAnchor) {
        verticalRotationOrigin = yAnchor;
    }

    SizingType Control::ZIndexSizing() const {
        return zIndexSizing;
    }

    void Control::ZIndexSizing(SizingType sz) {
        zIndexSizing = sz;
    }

    SizingType Control::RotationOriginSizing() const {
        return rotationOriginSizing;
    }

    void Control::RotationOriginSizing(SizingType sz) {
        rotationOriginSizing = sz;
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
         OnSizeChanged();
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
        OnPositionChanged();
    }

    Netcode::Float2 Control::RotationOrigin() const
    {
        return rotationOrigin;
    }

    void Control::RotationOrigin(const Netcode::Float2 & pos) {
        RotationOriginSizing(SizingType::FIXED);
        rotationOrigin = pos;
    }

    void Control::RotationOrigin(HorizontalAnchor x, VerticalAnchor y)
    {
        HorizontalRotationOrigin(x);
        VerticalRotationOrigin(y);
        rotationOrigin = CalculateAnchorOffset(HorizontalRotationOrigin(), VerticalRotationOrigin(), Size());
        RotationOriginSizing(SizingType::DERIVED);
    }

    /*
    for now, every Control rotates on its own, not affecting its children.
    */
    Netcode::Float2 Control::ScreenPosition() const
    {
        Netcode::Vector2 parentScreenPos = Netcode::Float2::Zero;
        Netcode::Vector2 anchorOffs = Netcode::Float2::Zero;
        Netcode::Vector2 anchorDiff = Netcode::Float2::Zero;
        Netcode::Vector2 parentRotationOrigin = Netcode::Float2::Zero;

        if(parent != nullptr) {
            parentRotationOrigin = parent->RotationOrigin();
            parentScreenPos = parent->ScreenPosition();
            parentScreenPos -= parentRotationOrigin;
            anchorOffs = CalculateAnchorOffset(parent->HorizontalContentAlignment(), parent->VerticalContentAlignment(), parent->Size());
            anchorDiff = CalculateAnchorOffset(parent->HorizontalContentAlignment(), parent->VerticalContentAlignment(), BoxSize());
        }

        return parentScreenPos + anchorOffs - anchorDiff + Position() + RotationOrigin();
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

    void Control::UpdateSize(const Netcode::Float2 & screenSize)
    {
        for(auto & child : children) {
            child->UpdateSize(screenSize);
        }

        if(Sizing() == SizingType::WINDOW) {
            Size(screenSize);
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

    void Control::UpdateLayout() {
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

    void Control::ZIndex(float zValue) {
        ZIndexSizing(SizingType::FIXED);
        zIndex = zValue;
    }

    float Control::ZIndex() const {
        return zIndex;
    }

    void Control::ResetZIndex() {
        ZIndex(0.0f);
        ZIndexSizing(SizingType::DERIVED);
    }

    void Control::AddChild(std::shared_ptr<Control> child) {
        children.push_back(child);
        child->Parent(shared_from_this());
    }

    void Control::Update(float dt)
    {
        animations.Update(dt);

        for(auto & child : children) {
            child->Update(dt);
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
        UpdateZIndices(1);
    }

    void Control::OnKeyDown(KeyboardEventArgs & args) {

    }

    void Control::OnKeyUp(KeyboardEventArgs & args) {

    }

    void Control::OnMouseEnter(MouseEventArgs & args) {
        MouseEnterEvent.Invoke(this, args);
    }

    void Control::OnMouseMove(MouseEventArgs & args) {
        MouseMoveEvent.Invoke(this, args);
    }

    void Control::OnMouseLeave(MouseEventArgs & args) {
        MouseLeaveEvent.Invoke(this, args);
    }

    void Control::Render(Netcode::SpriteBatchPtr batch) {
        for(auto & child : children) {
            child->Render(batch);
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
        if(RotationOriginSizing() == SizingType::DERIVED) {
            rotationOrigin = CalculateAnchorOffset(HorizontalRotationOrigin(), VerticalRotationOrigin(), Size());
        }
    }

    void Control::OnPositionChanged() {
        for(auto & child : children) {
            child->OnPositionChanged();
        }
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
