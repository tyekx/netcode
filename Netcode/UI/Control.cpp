#include "Control.h"
#include <NetcodeFoundation/Exceptions.h>
#include <physx/PxRigidDynamic.h>
#include <physx/PxScene.h>

namespace Netcode::UI {


    Control::Control(const AllocType & allocator) : std::enable_shared_from_this<Control>{},
        size{ Float2::Zero },
        position{ Float2::Zero },
        screenPositionCache{ Float2::Zero },
        rotationOrigin{ Float2::Zero },
        margin{ Float4::Zero },
        padding{ Float4::Zero },
        rotationZ{ 0.0f },
        zIndex{ 0.0f },
        zIndexSizing{ SizingType::DERIVED },
        rotationOriginSizing{ SizingType::FIXED },
        sizing{ SizingType::FIXED },
        overflow { OverflowType::VISIBLE },
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
        OnMouseKeyPressed{ allocator },
        OnMouseKeyReleased{ allocator },
        OnClick{ allocator },
        OnMouseScroll{ allocator },
        OnFocused{ allocator },
        OnBlurred{ allocator },
        OnKeyPressed{ allocator },
        OnCharInput{ allocator },
        OnDrag{ allocator },
        OnEnabled{ allocator },
        OnDisabled{ allocator },
        OnParentChanged{ allocator },
        OnPositionChanged{ allocator },
        OnSizeChanged{ allocator },
        OnAnimationsFinished{ allocator } {

    }

    void Control::AssignActor(PxPtr<physx::PxRigidDynamic> actor) {
        physx::PxScene * scenePtr = actor->getScene();

        UndefinedBehaviourAssertion(scenePtr != nullptr);
        UndefinedBehaviourAssertion(pxActor == nullptr);

        actor->userData = this;

        pxActor = std::move(actor);

        lastScenePtr = nullptr;
    }

    void Control::PropagateOnKeyPressed(KeyEventArgs & args)
    {
        if(args.Handled()) {
            args.HandledBy(this);
            OnKeyPressed.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnKeyPressed(args);
            }
        }
    }

    Control::Control(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor) : Control{ allocator } {
        if(pxActor != nullptr) {
            AssignActor(std::move(pxActor));
        }
    }

    OverflowType Control::Overflow() const {
        return overflow;
    }

    void Control::Overflow(OverflowType ot) {
        overflow = ot;
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

    Float2 Control::CalculateScreenPosition() {
        Vector2 parentScreenPos = Float2::Zero;
        Vector2 anchorOffs = Float2::Zero;
        Vector2 anchorDiff = Float2::Zero;
        Vector2 parentRotationOrigin = Float2::Zero;

        if(parent != nullptr) {
            parentRotationOrigin = parent->RotationOrigin();
            parentScreenPos = parent->ScreenPosition();
            parentScreenPos -= parentRotationOrigin;
            anchorOffs = CalculateAnchorOffset(parent->HorizontalContentAlignment(), parent->VerticalContentAlignment(), parent->Size());
            anchorDiff = CalculateAnchorOffset(parent->HorizontalContentAlignment(), parent->VerticalContentAlignment(), BoxSize());
        }

        return parentScreenPos + anchorOffs - anchorDiff + Position() + RotationOrigin();
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

    Float2 Control::CalculateAnchorOffset(HorizontalAnchor xAnchor, VerticalAnchor yAnchor, const Float2 & controlSize) {
        Float2 anchorOffset;

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
        if(pxActor != nullptr) {
            Vector2 sPos = ScreenPosition();
            Vector2 halfSize = Vector2{ Size() } / 2.0f;

            Float2 sp = sPos + halfSize;

            pxActor->setGlobalPose(physx::PxTransform{ sp.x, sp.y, ZIndex(), physx::PxQuat{ RotationZ(), physx::PxVec3{ 0.0f, 0.0f, 1.0f } } });
        }
    }

    void Control::UpdateActorShape() {
        if(pxActor != nullptr) {
            physx::PxShape * shape{ nullptr };
            physx::PxU32 returnedShapes = pxActor->getShapes(&shape, 1, 0);

            if(returnedShapes == 1) {
                Vector2 vSize = Size();

                if(vSize.AnyZero()) {
                    if(lastScenePtr == nullptr) {
                        lastScenePtr = pxActor->getScene();
                        lastScenePtr->removeActor(*pxActor);
                    }
                } else {
                    if(lastScenePtr != nullptr) {
                        lastScenePtr->addActor(*pxActor);
                        lastScenePtr = nullptr;
                    }

                    Float2 halfSize = vSize / 2.0f;

                    shape->setGeometry(physx::PxBoxGeometry{ halfSize.x, halfSize.y, 0.25f });

                    UpdateActorPose();
                }
            }
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

    void Control::AlignChildren() {
        for(auto & i : children) {
            i->PropagateOnPositionChanged();
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

    Float2 Control::Size() const {
        return size;
    }

    void Control::Size(const Float2 & sz) {
        size = sz;
        UpdateActorShape();
        PropagateOnSizeChanged();
    }

    Float2 Control::BoxSize() const
    {
        Float2 cSize = CalculatedSize();
        Float4 m = Margin();

        return Float2{ cSize.x + m.x + m.z, cSize.y + m.y + m.w };
    }

    Float2 Control::CalculatedSize() const
    {
        Vector2 s = Size();
        Float4 p = Padding();

        Vector2 horizontal = Float2{ p.x + p.z, 0.0f };
        Vector2 vertical = Float2{ 0.0f, p.y + p.w };

        return s + horizontal + vertical;
    }

    Float2 Control::Position() const {
        return position;
    }

    void Control::Position(const Float2 & pos) {
        position = pos;
        PropagateOnPositionChanged();
    }

    Float2 Control::RotationOrigin() const
    {
        return rotationOrigin;
    }

    void Control::RotationOrigin(const Float2 & pos) {
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
    Float2 Control::ScreenPosition() const
    {
        return screenPositionCache;
    }

    Float4 Control::Margin() const {
        return margin;
    }

    void Control::Margin(const Float4 & leftTopRightBottom) {
        margin = leftTopRightBottom;
    }

    Float4 Control::Padding() const {
        return padding;
    }

    void Control::Padding(const Float4 & leftTopRightBottom) {
        padding = leftTopRightBottom;
    }

    void Control::UpdateLayout() {
        /**
        * Every control will get the UpdateLayout call, so invoking
        * hierarchy propagation is not necessary
        */

        if(Sizing() == SizingType::INHERITED) {
            UndefinedBehaviourAssertion(parent != nullptr);

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
        
        PropagateOnPositionChanged();
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

        if(animations.Empty() && !OnAnimationsFinished.Empty()) {
            OnAnimationsFinished.Invoke(this);
            OnAnimationsFinished.Clear();
        }

        for(auto & child : children) {
            child->Update(dt);
        }
    }

    void Control::PropagateOnCharInput(CharInputEventArgs & args)
    {
        if(args.Handled()) {
            args.HandledBy(this);
            OnCharInput.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnCharInput(args);
            }
        }
    }

    void Control::PropagateOnFocused(FocusChangedEventArgs & args) {
        if(args.Handled()) {
            args.HandledBy(this);
            OnFocused.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnFocused(args);
            }
        }
    }

    void Control::PropagateOnBlurred(FocusChangedEventArgs & args)
    {
        if(args.Handled()) {
            args.HandledBy(this);
            OnBlurred.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnBlurred(args);
            }
        }
    }

    void Control::PropagateOnMouseKeyPressed(MouseEventArgs & args) {
        if(args.Handled()) {
            args.HandledBy(this);
            OnMouseKeyPressed.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnMouseKeyPressed(args);
            }
        }
    }

    void Control::PropagateOnMouseKeyReleased(MouseEventArgs & args) {
        if(args.Handled()) {
            args.HandledBy(this);
            OnMouseKeyReleased.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnMouseKeyReleased(args);
            }
        }
    }

    void Control::PropagateOnDrag(DragEventArgs & args)
    {
        if(args.Handled()) {
            args.HandledBy(this);
            OnDrag.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnDrag(args);
            }
        }
    }

    Float2 Control::DeriveSize()
    {
        Vector2 maxSize = Float2::Zero;

        for(auto & child : children) {
            maxSize = maxSize.Max(child->BoxSize());
        }

        return maxSize;
    }

    void Control::UpdateZIndices() {
        UpdateZIndices(ZIndex() + 1.0f);
    }

    void Control::CheckSizingConsistency() {
        if(parent != nullptr) {
            if(parent->Sizing() == SizingType::DERIVED && Sizing() == SizingType::INHERITED) {
                UndefinedBehaviourAssertion(false);
            }
        }
    }

    void Control::Render(SpriteBatchPtr batch) {
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

            parent->AlignChildren();
        }

        // sanity check since DERIVED -> INHERITED is disallowed
        if(Sizing() != SizingType::DERIVED) {
            for(auto & child : children) {
                if(child->Sizing() == SizingType::INHERITED) {
                    child->PropagateOnSizeChanged();
                }
            }
        }

        OnSizeChanged.Invoke(this);
    }

    void Control::PropagateOnParentChanged() {
        CheckSizingConsistency();

        OnParentChanged.Invoke(this);
    }

    void Control::PropagateOnPositionChanged() {
        screenPositionCache = CalculateScreenPosition();

        UpdateActorPose();

        for(auto & child : children) {
            child->PropagateOnPositionChanged();
        }

        OnPositionChanged.Invoke(this);
    }

    void Control::PropagateOnMouseEnter(MouseEventArgs & args) {
        if(args.Handled()) {
            args.HandledBy(this);
            OnMouseEnter.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnMouseEnter(args);
            }
        }
    }

    void Control::PropagateOnMouseMove(MouseEventArgs & args) {
        if(hoverState == HoverState::INACTIVE) {
            MouseEventArgs copyArgs{ args.Position(), args.Key(), args.Modifier() };
            PropagateOnMouseEnter(copyArgs);
        }

        hoverState = HoverState::RAYCASTED;

        if(args.Handled()) {
            args.HandledBy(this);
            OnMouseMove.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnMouseMove(args);
            }
        }
    }

    void Control::PropagateOnMouseLeave(MouseEventArgs & args) {
        if(hoverState == HoverState::RAYCASTED) {
            return;
        }

        if(hoverState == HoverState::HOVERED) {
            hoverState = HoverState::INACTIVE;

            OnMouseLeave.Invoke(this, args);

            for(auto & child : children) {
                child->PropagateOnMouseLeave(args);
            }
        }
    }

    void Control::PropagateOnClick(MouseEventArgs & args) {
        OnClick.Invoke(this, args);
        if(args.Handled() || parent == nullptr) {
            args.Handled(true);
            args.HandledBy(this);
        } else {
            parent->PropagateOnClick(args);
        }
    }

    void Control::PropagateOnMouseScroll(ScrollEventArgs & args) {
        if(args.Handled()) {
            args.HandledBy(this);
            OnMouseScroll.Invoke(this, args);
        } else {
            if(parent != nullptr) {
                parent->PropagateOnMouseScroll(args);
            }
        }
    }

}
