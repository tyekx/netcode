#pragma once

#include <NetcodeFoundation/Memory.h>

#include "Common.h"
#include "Animation.hpp"

namespace Netcode::UI {

    class Control : public std::enable_shared_from_this<Control> {
    protected:

        enum class HoverState {
            INACTIVE, HOVERED, RAYCASTED
        };

        Netcode::Float2 size;
        Netcode::Float2 position;
        Netcode::Float2 rotationOrigin;
        Netcode::Float4 margin;
        Netcode::Float4 padding;
        float rotationZ;
        float zIndex;
        SizingType zIndexSizing;
        SizingType rotationOriginSizing;
        SizingType sizing;
        HorizontalAnchor horizontalRotationOrigin;
        VerticalAnchor verticalRotationOrigin;
        HorizontalAnchor horizontalContentAlignment;
        VerticalAnchor verticalContentAlignment;
        physx::PxScene * lastScenePtr;
        Netcode::PxPtr<physx::PxRigidDynamic> pxActor;
        std::shared_ptr<Control> parent;
        std::vector<std::shared_ptr<Control>> children;
        AnimationContainer animations;
        HoverState hoverState;
        bool enabled;

        static HoverState DecayState(HoverState hState) {
            if(hState == HoverState::RAYCASTED) {
                return HoverState::HOVERED;
            }
            return HoverState::INACTIVE;
        }

    public:

        static constexpr float MAX_DEPTH = 256.0f;

        using AllocType = Netcode::Memory::StdAllocatorAdapter<void, Netcode::Memory::ObjectAllocator>;

        template<typename ... U>
        using EventType = Netcode::ManagedEvent<AllocType, U...>;

        EventType<Control *, MouseEventArgs &> OnMouseEnter;
        EventType<Control *, MouseEventArgs &> OnMouseLeave;
        EventType<Control *, MouseEventArgs &> OnMouseMove;
        EventType<Control *, MouseEventArgs &> OnMouseClick;
        EventType<Control *, ScrollEventArgs &> OnMouseScroll;
        EventType<Control *, FocusChangedEventArgs &> OnFocused;
        EventType<Control *> OnEnabled;
        EventType<Control *> OnDisabled;
        EventType<Control *> OnParentChanged;
        EventType<Control *> OnPositionChanged;
        EventType<Control *> OnSizeChanged;

    protected:

        SizingType ZIndexSizing() const;

        void ZIndexSizing(SizingType sz);

        SizingType RotationOriginSizing() const;

        void RotationOriginSizing(SizingType sz);

        static Netcode::Float2 CalculateAnchorOffset(HorizontalAnchor xAnchor, VerticalAnchor yAnchor, const Netcode::Float2 & controlSize);

        void UpdateZIndices(float depth);

        void UpdateActorPose();

        void UpdateActorShape();

        void AddActorToScene();

        void RemoveActorFromScene();

        /**
        * Invoked when the Sizing is derived and the layout is being updated
        */
        virtual Netcode::Float2 DeriveSize();

        /**
        * Internal event
        * Way to update children position
        */
        virtual void OnLayoutChanged();

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

        void AssignActor(Netcode::PxPtr<physx::PxRigidDynamic> actor);

    public:
        virtual void PropagateOnFocused(FocusChangedEventArgs & args);

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

        Control(const AllocType & allocator, Netcode::PxPtr<physx::PxRigidDynamic> pxActor);

        virtual ~Control() = default;

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

        const AnimationContainer & Animations() const {
            return animations;
        }

        bool Enabled() const;

        void Enabled(bool isEnabled);

        SizingType Sizing() const;

        void Sizing(SizingType sizingType);

        Netcode::Float2 Size() const;

        void Size(const Netcode::Float2 & sz);

        Netcode::Float2 Position() const;

        void Position(const Netcode::Float2 & pos);

        Netcode::Float2 RotationOrigin() const;

        void RotationOrigin(const Netcode::Float2 & pos);

        void RotationOrigin(HorizontalAnchor x, VerticalAnchor y);

        Netcode::Float2 ScreenPosition() const;

        Netcode::Float4 Margin() const;

        void Margin(const Netcode::Float4 & leftTopRightBottom);

        Netcode::Float4 Padding() const;

        void Padding(const Netcode::Float4 & leftTopRightBottom);

        void Parent(std::shared_ptr<Control> newParent);

        std::shared_ptr<Control> Parent() const;

        const std::vector<std::shared_ptr<Control>> & Children() const;

        void RotationZ(float rotZ);

        float RotationZ() const;

        void ZIndex(float zValue);

        float ZIndex() const;

        void ResetZIndex();

        virtual Netcode::Float2 BoxSize() const;

        virtual Netcode::Float2 CalculatedSize() const;

        virtual void UpdateZIndices();

        virtual void UpdateLayout();

        virtual void Update(float dt);

        virtual void Destruct();

        virtual void AddChild(std::shared_ptr<Control> child);

        virtual void Render(Netcode::SpriteBatchPtr batch);

    };

}
