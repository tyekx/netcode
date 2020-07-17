#include "Control.h"
#include <NetcodeFoundation/Exceptions.h>

namespace Netcode::UI {


    Control::Control(const AllocType & allocator) : std::enable_shared_from_this<Control>{},
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
        lastScenePtr{ nullptr },
        pxActor{ nullptr },
        parent{ nullptr },
        children{},
        animations{},
        hoverState{ HoverState::INACTIVE },
        enabled{ true },
        OnMouseEnter{ allocator },
        OnMouseLeave{ allocator },
        OnMouseMove{ allocator },
        OnMouseClick{ allocator },
        OnMouseScroll{ allocator },
        OnFocused{ allocator },
        OnEnabled{ allocator },
        OnDisabled{ allocator },
        OnParentChanged{ allocator },
        OnPositionChanged{ allocator },
        OnSizeChanged{ allocator } {

    }

    void Control::AssignActor(Netcode::PxPtr<physx::PxRigidDynamic> actor) {
        physx::PxScene * scenePtr = actor->getScene();

        Netcode::UndefinedBehaviourAssertion(scenePtr != nullptr);
        Netcode::UndefinedBehaviourAssertion(pxActor == nullptr);

        actor->userData = this;

        pxActor = std::move(actor);
    }

    Control::Control(const AllocType & allocator, Netcode::PxPtr<physx::PxRigidDynamic> pxActor) : Control{ allocator } {
        AssignActor(std::move(pxActor));
    }

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

    void Control::UpdateZIndices(float depth) {
        if(ZIndexSizing() == SizingType::DERIVED) {
            zIndex = depth;
            for(auto & child : children) {
                child->UpdateZIndices(depth + 1.0f);
            }
        } else {
            for(auto & child : children) {
                child->UpdateZIndices(ZIndex() + 1.0f);
            }
        }
    }

    void Control::UpdateActorPose() {
        Netcode::Vector2 sPos = ScreenPosition();
        Netcode::Vector2 halfSize = Netcode::Vector2{ Size() } / 2.0f;

        Netcode::Float2 sp = sPos + halfSize;

        pxActor->setGlobalPose(physx::PxTransform{ sp.x, sp.y, ZIndex(), physx::PxQuat{ RotationZ(), physx::PxVec3{ 0.0f, 0.0f, 1.0f } } });
    }

    void Control::UpdateActorShape() {
        physx::PxShape * shape{ nullptr };
        physx::PxU32 returnedShapes = pxActor->getShapes(&shape, 1, 0);

        if(returnedShapes == 1) {
            Netcode::Float2 halfSize = Netcode::Vector2{ Size() } / 2.0f;

            shape->setGeometry(physx::PxBoxGeometry{ halfSize.x, halfSize.y, 0.25f });

            UpdateActorPose();
        }
    }

    void Control::AddActorToScene()
    {
        if(lastScenePtr != nullptr) {
            UpdateActorShape();
            UpdateActorPose();
            lastScenePtr->addActor(*pxActor);
            lastScenePtr = nullptr;
        }
    }

    void Control::RemoveActorFromScene()
    {
        if(pxActor != nullptr) {
            if(lastScenePtr == nullptr) {
                lastScenePtr = pxActor->getScene();

                if(lastScenePtr != nullptr) {
                    lastScenePtr->removeActor(*pxActor);
                }
            }
        }
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

    bool Control::Enabled() const {
        return enabled;
    }

    void Control::Enabled(bool isEnabled) {
        if(isEnabled && Enabled() != isEnabled) {
            enabled = isEnabled;
            PropagateOnEnabled();
        }

        if(!isEnabled && Enabled() != isEnabled) {
            enabled = isEnabled;
            PropagateOnDisabled();
        }
    }

    SizingType Control::Sizing() const {
        return sizing;
    }

    void Control::Sizing(SizingType sizingType) {
        sizing = sizingType;
        CheckSizingConsistency();
    }

    Netcode::Float2 Control::Size() const {
        return size;
    }

    void Control::Size(const Netcode::Float2 & sz) {
        size = sz;
        UpdateActorShape();
        PropagateOnSizeChanged();
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
        PropagateOnPositionChanged();
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

    /*
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
    }*/

    void Control::UpdateLayout() {
        /**
        * Every control will get the UpdateLayout call, so invoking
        * hierarchy propagation is not necessary
        */

        if(Sizing() == SizingType::INHERITED) {
            Netcode::UndefinedBehaviourAssertion(parent != nullptr);

            size = parent->Size();

            UpdateActorShape();

            OnSizeChanged.Invoke(this);
        }

        for(auto & child : children) {
            child->UpdateLayout();
        }

        if(Sizing() == SizingType::DERIVED) {
            size = DeriveSize();

            UpdateActorShape();
        }
    }

    void Control::Destruct() {
        Parent(nullptr);

        RemoveActorFromScene();

        pxActor.Reset(nullptr);

        for(auto & child : children) {
            child->Destruct();
        }

        children.clear();
    }

    void Control::Parent(std::shared_ptr<Control> newParent) {
        if(parent != newParent) {
            parent = newParent;
            PropagateOnParentChanged();
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

    void Control::PropagateOnFocused(FocusChangedEventArgs & args) {
        if(args.Handled()) {
            OnFocused.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnFocused(args);
            }
        }
    }

    Netcode::Float2 Control::DeriveSize()
    {
        Netcode::Vector2 maxSize = Netcode::Float2::Zero;

        for(auto & child : children) {
            maxSize = maxSize.Max(child->BoxSize());
        }

        return maxSize;
    }

    void Control::OnLayoutChanged() {

    }

    void Control::UpdateZIndices() {
        UpdateZIndices(ZIndex() + 1.0f);
    }

    void Control::CheckSizingConsistency() {
        if(parent != nullptr) {
            if(parent->Sizing() == SizingType::DERIVED && Sizing() == SizingType::INHERITED) {
                Netcode::UndefinedBehaviourAssertion(false);
            }
        }
    }

    void Control::Render(Netcode::SpriteBatchPtr batch) {
        for(auto & child : children) {
            child->Render(batch);
        }
    }

    void Control::PropagateOnEnabled() {
        if(!Enabled()) {
            return;
        }

        OnEnabled.Invoke(this);

        for(auto & child : children) {
            child->PropagateOnEnabled();
        }
    }

    void Control::PropagateOnDisabled() {
        OnDisabled.Invoke(this);

        for(auto & child : children) {
            child->PropagateOnDisabled();
        }
    }

    void Control::PropagateOnSizeChanged() {
        if(RotationOriginSizing() == SizingType::DERIVED) {
            rotationOrigin = CalculateAnchorOffset(HorizontalRotationOrigin(), VerticalRotationOrigin(), Size());
        }

        if(parent != nullptr) {
            if(parent->Sizing() == SizingType::DERIVED) {
                parent->PropagateOnSizeChanged();
            }
        }

        for(auto & child : children) {
            if(child->Sizing() == SizingType::INHERITED) {
                child->PropagateOnSizeChanged();
            }
        }

        OnSizeChanged.Invoke(this);
    }

    void Control::PropagateOnParentChanged() {
        CheckSizingConsistency();

        OnParentChanged.Invoke(this);
    }

    void Control::PropagateOnPositionChanged() {
        UpdateActorPose();

        for(auto & child : children) {
            child->PropagateOnPositionChanged();
        }

        OnPositionChanged.Invoke(this);
    }

    void Control::PropagateOnMouseEnter(MouseEventArgs & args) {
        if(args.Handled()) {
            OnMouseEnter.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnMouseEnter(args);
            }
        }
    }

    void Control::PropagateOnMouseMove(MouseEventArgs & args) {
        if(hoverState == HoverState::INACTIVE) {
            MouseEventArgs copyArgs{ args.Position() };
            PropagateOnMouseEnter(copyArgs);
        }

        hoverState = HoverState::RAYCASTED;

        if(args.Handled()) {
            OnMouseMove.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnMouseMove(args);
            }
        }
    }

    void Control::PropagateOnMouseLeave(MouseEventArgs & args) {
        HoverState decayedState = DecayState(hoverState);

        if(decayedState == HoverState::INACTIVE && hoverState != HoverState::INACTIVE) {
            OnMouseLeave.Invoke(this, args);
        }

        hoverState = decayedState;

        for(auto & child : children) {
            child->PropagateOnMouseLeave(args);
        }
    }

    void Control::PropagateOnClick(MouseEventArgs & args) {
        if(args.Handled()) {
            OnMouseClick.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnClick(args);
            }
        }
    }

    void Control::PropagateOnMouseScroll(ScrollEventArgs & args) {
        if(args.Handled()) {
            OnMouseScroll.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnMouseScroll(args);
            }
        }
    }

}
