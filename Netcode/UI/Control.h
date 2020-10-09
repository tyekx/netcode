#pragma once

#include <NetcodeFoundation/Memory.h>

#include "Common.h"
#include "Animation.hpp"
#include "../PxPtr.hpp"

namespace Netcode::UI {

    class Control : public std::enable_shared_from_this<Control> {
    protected:

        enum class HoverState {
            INACTIVE, HOVERED, RAYCASTED
        };

        Float2 size;
        Float2 position;
        Float2 screenPositionCache;
        Float2 rotationOrigin;
        Float4 margin;
        Float4 padding;
        float rotationZ;
        float zIndex;
        SizingType zIndexSizing;
        SizingType rotationOriginSizing;
        SizingType sizing;
        OverflowType overflow;
        HorizontalAnchor horizontalRotationOrigin;
        VerticalAnchor verticalRotationOrigin;
        HorizontalAnchor horizontalContentAlignment;
        VerticalAnchor verticalContentAlignment;
        physx::PxScene * lastScenePtr;
        PxPtr<physx::PxRigidDynamic> pxActor;
        Ref<Control> parent;
        std::vector<Ref<Control>> children;
        AnimationContainer animations;
        HoverState hoverState;
        bool enabled;

        Float2 CalculateScreenPosition();

    public:

        static constexpr float MAX_DEPTH = 256.0f;

        using AllocType = Memory::StdAllocatorAdapter<void, Memory::ObjectAllocator>;

        template<typename ... U>
        using EventType = ManagedEvent<AllocType, U...>;

        EventType<Control *, MouseEventArgs &> OnMouseEnter;
        EventType<Control *, MouseEventArgs &> OnMouseLeave;
        EventType<Control *, MouseEventArgs &> OnMouseMove;
        EventType<Control *, MouseEventArgs &> OnMouseKeyPressed;
        EventType<Control *, MouseEventArgs &> OnMouseKeyReleased;
        EventType<Control *, MouseEventArgs &> OnClick;
        EventType<Control *, ScrollEventArgs &> OnMouseScroll;
        EventType<Control *, FocusChangedEventArgs &> OnFocused;
        EventType<Control *, FocusChangedEventArgs &> OnBlurred;
        EventType<Control *, KeyEventArgs &> OnKeyPressed;
        EventType<Control *, CharInputEventArgs &> OnCharInput;
        EventType<Control *, DragEventArgs &> OnDrag;
        EventType<Control *> OnEnabled;
        EventType<Control *> OnDisabled;
        EventType<Control *> OnParentChanged;
        EventType<Control *> OnPositionChanged;
        EventType<Control *> OnSizeChanged;
        EventType<Control *> OnAnimationsFinished;

    protected:

        SizingType ZIndexSizing() const;

        void ZIndexSizing(SizingType sz);

        SizingType RotationOriginSizing() const;

        void RotationOriginSizing(SizingType sz);

        static Float2 CalculateAnchorOffset(HorizontalAnchor xAnchor, VerticalAnchor yAnchor, const Float2 & controlSize);

        void UpdateZIndices(float depth);

        void UpdateActorPose();

        void UpdateActorShape();

        void AddActorToScene();

        void RemoveActorFromScene();

        /**
        * Positions the children controls
        */
        virtual void AlignChildren();

        /**
        * Invoked when the Sizing is derived and the layout is being updated
        */
        virtual Float2 DeriveSize();

        /**
        * DERIVED -> INHERITED in the hierarchy chain is not allowed
        */
        void CheckSizingConsistency();

        /**
        * Downward pre propagated event
        */
        virtual void PropagateOnEnabled();

        /**
        * Downward pre propagated event
        */
        virtual void PropagateOnDisabled();

        /**
        * Mixed pre propagated event
        */
        virtual void PropagateOnSizeChanged();

        /**
        * Local pre propagated event
        */
        virtual void PropagateOnParentChanged();

        /**
        * Downward pre propagated event
        */
        virtual void PropagateOnPositionChanged();

        /**
        * Internal constructor for initializing the essentials
        */
        Control(const AllocType & allocator);

        void AssignActor(PxPtr<physx::PxRigidDynamic> actor);

        // implementation detail
        static void RaycastEnter(Control * ptr) {
            ptr->hoverState = HoverState::RAYCASTED;
        }

        // implementation detail
        static void RaycastFade(Control * ptr) {
            ptr->hoverState = HoverState::HOVERED;
        }

    public:
        virtual void PropagateOnCharInput(CharInputEventArgs & args);

        virtual void PropagateOnFocused(FocusChangedEventArgs & args);
        virtual void PropagateOnBlurred(FocusChangedEventArgs & args);

        virtual void PropagateOnMouseKeyPressed(MouseEventArgs & args);
        virtual void PropagateOnMouseKeyReleased(MouseEventArgs & args);

        /**
        * Local event
        */
        virtual void PropagateOnDrag(DragEventArgs & args);

        /**
        * Upward post propagated event
        * Invokes MouseClick if args was handled previously
        */
        virtual void PropagateOnClick(MouseEventArgs & args);

        /**
        * Upward post propagated event
        * Invokes MouseEnter if args was handled previously
        */
        virtual void PropagateOnMouseEnter(MouseEventArgs & args);

        /**
        * Upward post propagated event
        * Invokes MouseMove if args was handled previously
        */
        virtual void PropagateOnMouseMove(MouseEventArgs & args);

        /**
        * Upward post propagated event
        * Invokes MouseLeave if args was handled previously
        */
        virtual void PropagateOnMouseLeave(MouseEventArgs & args);

        /**
        * Upward post propagated event
        * Invokes MouseScroll if args was handled previously
        */
        virtual void PropagateOnMouseScroll(ScrollEventArgs & args);

        /**
        * Upward post propagated event
        * Invokes KeyPressed if args was handled previously
        */
        virtual void PropagateOnKeyPressed(KeyEventArgs & args);

        Control(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor);

        virtual ~Control() = default;

        OverflowType Overflow() const;

        void Overflow(OverflowType ot);

        HorizontalAnchor HorizontalRotationOrigin() const;

        void HorizontalRotationOrigin(HorizontalAnchor xAnchor);

        VerticalAnchor VerticalRotationOrigin() const;

        void VerticalRotationOrigin(VerticalAnchor yAnchor);

        HorizontalAnchor HorizontalContentAlignment() const;

        void HorizontalContentAlignment(HorizontalAnchor horizontalAnchor);

        VerticalAnchor VerticalContentAlignment() const;

        void VerticalContentAlignment(VerticalAnchor verticalAnchor);

        void AddAnimation(std::unique_ptr<Animation> anim) {
            anim->Run(0.0f);
            animations.Add(std::move(anim));
        }

        AnimationContainer & Animations() {
            return animations;
        }

        void ClearAnimations() {
            animations.Clear();
        }

        bool Hovered() const {
            return hoverState != HoverState::INACTIVE;
        }

        bool Enabled() const;

        void Enabled(bool isEnabled);

        SizingType Sizing() const;

        void Sizing(SizingType sizingType);

        Float2 Size() const;

        void Size(const Float2 & sz);

        float SizeX() const {
            return size.x;
        }

        void SizeX(float v) {
            Size(Float2{ v, size.y });
        }

        float SizeY() const {
            return size.y;
        }

        void SizeY(float v) {
            Size(Float2{ size.x, v });
        }

        Float2 Position() const;

        void Position(const Float2 & pos);

        Float2 RotationOrigin() const;

        void RotationOrigin(const Float2 & pos);

        void RotationOrigin(HorizontalAnchor x, VerticalAnchor y);

        Float2 ScreenPosition() const;

        Float4 Margin() const;

        void Margin(const Float4 & leftTopRightBottom);

        Float4 Padding() const;

        void Padding(const Float4 & leftTopRightBottom);

        void Parent(Ref<Control> newParent);

        Ref<Control> Parent() const;

        const std::vector<Ref<Control>> & Children() const;

        void ClearChildren();

        void RotationZ(float rotZ);

        float RotationZ() const;

        void ZIndex(float zValue);

        float ZIndex() const;

        void ResetZIndex();

        virtual Float2 BoxSize() const;

        virtual Float2 CalculatedSize() const;

        virtual void UpdateZIndices();

        virtual void UpdateLayout();

        virtual void Update(float dt);

        virtual void Destruct();

        virtual void AddChild(Ref<Control> child);

        virtual void RemoveChild(Ref<Control> child);

        virtual void Render(Ptr<SpriteBatch> batch);

    };

}
