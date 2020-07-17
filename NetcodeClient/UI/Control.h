#pragma once

#include <memory>

#include <Netcode/HandleTypes.h>
#include <Netcode/Graphics/GraphicsContexts.h>
#include <Netcode/Functions.h>
#include <Netcode/PhysXWrapper.h>
#include <Netcode/Event.hpp>

#include "EventArgs.h"
#include "Animation.hpp"

using Netcode::SpriteBatch;

namespace UI {

    enum class Direction : uint32_t {
        VERTICAL, HORIZONTAL
    };

	enum class HorizontalAnchor : uint32_t {
		LEFT, CENTER, RIGHT
	};

	enum class VerticalAnchor : uint32_t {
		TOP, MIDDLE, BOTTOM
	};

    enum class SizingType : uint32_t {
        FIXED, DERIVED, INHERITED
    };

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


    class Panel : public Control {
    protected:
        Netcode::BorderType borderType;
        float borderWidth;
        float borderRadius;
        Netcode::BackgroundType backgroundType;
        VerticalAnchor backgroundVerticalAlignment;
        HorizontalAnchor backgroundHorizontalAlignment;
        Netcode::ResourceViewsRef backgroundImage;
        Netcode::Float2 backgroundSize;
        Netcode::UInt2 backgroundImageSize;
        Netcode::Float4 backgroundColor;
        Netcode::Float4 borderColor;

        Netcode::BorderType BorderType() const {
            return borderType;
        }

        void BorderType(Netcode::BorderType type) {
            borderType = type;
        }

        Netcode::BackgroundType BackgroundType() const {
            return backgroundType;
        }

        void BackgroundType(Netcode::BackgroundType type) {
            backgroundType = type;
        }

        void BackgroundImageSize(const Netcode::UInt2 & imgSize) {
            backgroundImageSize = imgSize;
        }

        Netcode::SpriteDesc GetSpriteDesc() const {
            if(BackgroundType() == Netcode::BackgroundType::TEXTURE) {
                Netcode::Float2 bgSize = BackgroundSize();

                if(bgSize.x == 0.0f || bgSize.y == 0.0f) {
                    return Netcode::SpriteDesc{ BackgroundImage(), BackgroundImageSize(), BackgroundColor() };
                } else {
                    HorizontalAnchor ha = BackgroundHorizontalAlignment();
                    VerticalAnchor va = BackgroundVerticalAlignment();

                    Netcode::Float2 controlSize = Size();

                    Netcode::Vector2 anchorDiff = CalculateAnchorOffset(ha, va, controlSize);
                    Netcode::Vector2 anchorOffset = CalculateAnchorOffset(ha, va, bgSize);

                    Netcode::Vector2 topLeftCorner = anchorOffset - anchorDiff;
                    Netcode::Vector2 bottomRightCorner = topLeftCorner + controlSize;

                    Netcode::Float2 tl = topLeftCorner;
                    Netcode::Float2 br = bottomRightCorner;

                    Netcode::Rect sourceRectangle{
                        static_cast<int32_t>(tl.x),
                        static_cast<int32_t>(tl.y),
                        static_cast<int32_t>(br.x),
                        static_cast<int32_t>(br.y)
                    };

                    return Netcode::SpriteDesc{ BackgroundImage(), BackgroundImageSize(), sourceRectangle, BackgroundColor() };
                }
            }

            if(BackgroundType() == Netcode::BackgroundType::SOLID) {
                return Netcode::SpriteDesc{ BackgroundColor() };
            }

            return Netcode::SpriteDesc{};
        }

        Netcode::BorderDesc GetBorderDesc() const {
            if(BorderType() == Netcode::BorderType::SOLID) {
                return Netcode::BorderDesc{ BorderWidth(), BorderRadius(), BorderColor() };
            }

            return Netcode::BorderDesc{};
        }

    public:
        Netcode::UInt2 BackgroundImageSize() const {
            return backgroundImageSize;
        }

        Panel(const AllocType & alloc, Netcode::PxPtr<physx::PxRigidDynamic> pxActor) : Control{ alloc, std::move(pxActor) }, borderType{ Netcode::BorderType::NONE }, borderWidth{ 0.0f }, borderRadius{ 0.0f }, backgroundType{ Netcode::BackgroundType::NONE }, backgroundVerticalAlignment{ VerticalAnchor::TOP }, backgroundHorizontalAlignment{ HorizontalAnchor::LEFT },
            backgroundImage{ nullptr }, backgroundSize{ Netcode::Float2::Zero }, backgroundImageSize{ Netcode::UInt2::Zero }, backgroundColor{ Netcode::Float4::Zero }, borderColor{ Netcode::Float4::Zero } { }

        virtual ~Panel() = default;

        VerticalAnchor BackgroundVerticalAlignment() const {
            return backgroundVerticalAlignment;
        }

        void BackgroundVerticalAlignment(VerticalAnchor verticalAlignment) {
            backgroundVerticalAlignment = verticalAlignment;
        }

        HorizontalAnchor BackgroundHorizontalAlignment() const {
            return backgroundHorizontalAlignment;
        }

        void BackgroundHorizontalAlignment(HorizontalAnchor horizontalAlignment) {
            backgroundHorizontalAlignment = horizontalAlignment;
        }

        Netcode::Float4 BackgroundColor() const {
            return backgroundColor;
        }

        void ResetBackground() {
            BackgroundImage(nullptr);
            BackgroundColor(Netcode::Float4::Zero);
            BackgroundSize(Netcode::Float2::Zero);
            BackgroundType(Netcode::BackgroundType::NONE);
        }

        void ResetBorder() {
            BorderRadius(0.0f);
            BorderWidth(0.0f);
            BorderColor(Netcode::Float4::Zero);
            BorderType(Netcode::BorderType::NONE);
        }

        float BorderRadius() const {
            return borderRadius;
        }

        void BorderRadius(float br) {
            borderRadius = br;
            BorderType(Netcode::BorderType::SOLID);
        }

        void BorderWidth(float bw) {
            borderWidth = bw;
            BorderType(Netcode::BorderType::SOLID);
        }

        float BorderWidth() const {
            return borderWidth;
        }

        void BorderColor(const Netcode::Float4 & c) {
            borderColor = c;
            BorderType(Netcode::BorderType::SOLID);
        }

        Netcode::Float4 BorderColor() const {
            return borderColor;
        }

        Netcode::Float2 BackgroundSize() const {
            return backgroundSize;
        }

        void BackgroundSize(const Netcode::Float2 & bgSizeInPixels) {
            backgroundSize = bgSizeInPixels;
        }

        void BackgroundColor(const Netcode::Float4 & color) {
            if(BackgroundType() == Netcode::BackgroundType::NONE) {
                BackgroundType(Netcode::BackgroundType::SOLID);
            }

            if(color.w == 0.0f) {
                BackgroundType(Netcode::BackgroundType::NONE);
            }

            backgroundColor = color;
        }

        float Opacity() const {
            return backgroundColor.w;
        }

        void Opacity(float w) {
            backgroundColor.w = w;
        }

        Netcode::ResourceViewsRef BackgroundImage() const {
            return backgroundImage;
        }

        void BackgroundImage(std::nullptr_t) {
            BackgroundImage(nullptr, Netcode::UInt2::Zero);
            BackgroundType(Netcode::BackgroundType::SOLID);
        }
        
        void BackgroundImage(Netcode::ResourceViewsRef imageRef, const Netcode::UInt2 & imageSize) {
            backgroundImage = imageRef;
            BackgroundImageSize(imageSize);
            if(imageRef != nullptr) {
                BackgroundType(Netcode::BackgroundType::TEXTURE);
            }
        }

        virtual void Render(Netcode::SpriteBatchPtr batch) override {
            Netcode::SpriteDesc spriteDesc = GetSpriteDesc();
            Netcode::BorderDesc borderDesc = GetBorderDesc();

            if(!spriteDesc.IsEmpty() || !borderDesc.IsEmpty()) {
                batch->DrawSprite(spriteDesc, borderDesc, ScreenPosition(), Size(), RotationOrigin(), RotationZ(), ZIndex());
            }

            Control::Render(batch);
        }
    };

    class StackPanel : public Panel {
    protected:
        Direction stackDirection;

        static Netcode::Float2 ZeroDirection(const Netcode::Float2 & value, Direction dir) {
            if(dir == Direction::VERTICAL) {
                return Netcode::Float2{ value.x, 0.0f };
            } else {
                return Netcode::Float2{ 0.0f, value.y };
            }
        }

        static Direction OppositeDirection(Direction dir) {
            if(dir == Direction::VERTICAL) {
                return Direction::HORIZONTAL;
            } else {
                return Direction::VERTICAL;
            }
        }

    public:

        virtual ~StackPanel() = default;

        StackPanel(const AllocType & allocator, Netcode::PxPtr<physx::PxRigidDynamic> pxActor) : Panel{ allocator, std::move(pxActor) }, stackDirection{ Direction::VERTICAL } {
            
        }

        Direction StackDirection() const {
            return stackDirection;
        }

        void StackDirection(Direction dir) {
            stackDirection = dir;
        }

        virtual void UpdateLayout() override {
            Panel::UpdateLayout();

            const Netcode::Vector2 mask = (StackDirection() == Direction::VERTICAL) ? Netcode::Float2::UnitX : Netcode::Float2::UnitY;
            const Netcode::Vector2 invMask = mask.Swizzle<1, 0>();
            const Netcode::Vector2 anchorOffset = mask * CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), Size());
            
            Netcode::Vector2 dirSum = Netcode::Float2::Zero;

            for(auto & child : children) {
                const Netcode::Float2 bs = child->BoxSize();
                const Netcode::Vector2 anchorDiff = mask * CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), bs);

                child->Position(anchorOffset - anchorDiff + dirSum);

                dirSum += invMask * bs;
            }
        }

        virtual Netcode::Float2 DeriveSize() override {
            Netcode::Vector2 mask = (StackDirection() == Direction::VERTICAL) ? Netcode::Float2::UnitY : Netcode::Float2::UnitX;
            Netcode::Vector2 invMask = mask.Swizzle<1, 0>();
            Netcode::Vector2 derivedSize = Netcode::Float2::Zero;

            for(auto & child : children) {
                Netcode::Vector2 childSize = child->BoxSize();

                derivedSize = derivedSize.Max(invMask * childSize);
                derivedSize += mask * childSize;
            }

            return derivedSize;
        }
    };

    class InputGroup : public StackPanel {
    protected:
    public:
        virtual ~InputGroup() = default;

        using StackPanel::StackPanel;
        
        /*
        handle tab key press
        */

        virtual void AddChild(std::shared_ptr<Control> child) override {
            if(!children.empty()) {
                children.clear();
            }
            StackPanel::AddChild(child);
        }
    };

    class ScrollViewer : public Panel {
    protected:
    public:
        virtual ~ScrollViewer() = default;

        using Panel::Panel;
    };

    class Label : public Panel {
    protected:
        Netcode::SpriteFontRef font;
        Netcode::Float4 textColor;
        std::wstring text;
        Netcode::Float2 textPosition;


        void TextPosition(const Netcode::Float2 & tp) {
            textPosition = tp;
        }

        Netcode::Float2 TextPosition() const {
            return textPosition;
        }

        void UpdateTextPosition() {
            const Netcode::Float2 textSize = font->MeasureString(text.c_str());
            const Netcode::Vector2 textAnchorOffset = CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), Size());
            const Netcode::Vector2 textAnchorDiff = CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), textSize);
            TextPosition(textAnchorOffset - textAnchorDiff);
        }

        void UpdateTextSize() {
            if(Sizing() == SizingType::DERIVED) {
                Size(font->MeasureString(text.c_str()));
                TextPosition(Netcode::Float2::Zero);
            } else {
                if(text.empty()) {
                    TextPosition(Netcode::Float2::Zero);
                } else {
                    UpdateTextPosition();
                }
            }
        }

    public:
        virtual ~Label() = default;

        Label(const AllocType & allocator, Netcode::PxPtr<physx::PxRigidDynamic> pxActor) : Panel{ allocator, std::move(pxActor) }, font{ nullptr }, textColor{ Netcode::Float4::Zero }, text{}, textPosition{ Netcode::Float2::Zero } {

        }

        void Font(Netcode::SpriteFontRef ref);
        Netcode::SpriteFontRef Font() const;

        Netcode::Float4 TextColor() const {
            return textColor;
        }

        void TextColor(const Netcode::Float4 & color) {
            textColor = color;
        }

        Netcode::Float3 TextRGB() const {
            return Netcode::Float3{ textColor.x, textColor.y, textColor.z };
        }

        void TextRGB(const Netcode::Float3 & rgb) {
            TextColor(Netcode::Float4{ rgb.x, rgb.y, rgb.z, TextColor().w });
        }

        float TextOpacity() const {
            return textColor.w;
        }

        void TextOpacity(float w) {
            textColor.w = w;
        }

        const std::wstring & Text() const {
            return text;
        }

        void Text(const std::wstring & ws) {
            if(text != ws) {
                text = ws;
                OnTextChanged();
            }
        }

        virtual void Render(Netcode::SpriteBatchPtr batch) override {
            Panel::Render(batch);

            if(font != nullptr && !text.empty()) {
                const Netcode::Vector2 screenPos = ScreenPosition();
                const Netcode::Vector2 textPos = TextPosition();

                font->DrawString(batch, text, screenPos + textPos, TextColor(), RotationOrigin(), RotationZ(), ZIndex());
            }
        }

        virtual void OnFontChanged() {
            if(font != nullptr) {
                UpdateTextSize();
            }
        }

        virtual void OnTextChanged() {
            if(font != nullptr) {
                UpdateTextSize();
            }
        }
    };


    class Input : public Label {
    protected:
        int32_t tabIndex;
        bool focused;
        bool hovered;

        virtual void PropagateOnFocused(FocusChangedEventArgs & evtArgs) override {
            if(evtArgs.TabIndex() == TabIndex()) {
                evtArgs.Handled(true);
                focused = true;
            }
        }

    public:
        virtual ~Input() = default;
        Input(const AllocType & allocator, Netcode::PxPtr<physx::PxRigidDynamic> pxActor) : Label{ allocator, std::move(pxActor) }, tabIndex{ 0 }, focused{ false }, hovered{ false } {
            
        }

        int32_t TabIndex() const {
            return tabIndex;
        }

        void TabIndex(int32_t tbIndex) {
            tabIndex = tbIndex;
        }

        bool Focused() const {
            return focused;
        }

    };

    class Button : public Input {
    protected:

        virtual void PropagateOnMouseEnter(MouseEventArgs & evtArgs) override {
            evtArgs.Handled(true);
            Input::PropagateOnMouseEnter(evtArgs);
        }

        virtual void PropagateOnMouseLeave(MouseEventArgs & evtArgs) override {
            evtArgs.Handled(true);
            Input::PropagateOnMouseLeave(evtArgs);
        }

        virtual void PropagateOnMouseMove(MouseEventArgs & evtArgs) override {
            evtArgs.Handled(true);
            Input::PropagateOnMouseMove(evtArgs);
        }

        virtual void PropagateOnClick(MouseEventArgs & evtArgs) override {
            evtArgs.Handled(true);
            Input::PropagateOnClick(evtArgs);
        }

    public:
        virtual ~Button() = default;

        using Input::Input;
    };

    class TextBox : public Input {
    protected:
        bool isPassword;

        virtual void PropagateOnMouseEnter(MouseEventArgs & evtArgs) override {
            evtArgs.Handled(true);
            Input::PropagateOnMouseEnter(evtArgs);
        }

        virtual void PropagateOnMouseLeave(MouseEventArgs & evtArgs) override {
            evtArgs.Handled(true);
            Input::PropagateOnMouseLeave(evtArgs);
        }

        virtual void PropagateOnMouseMove(MouseEventArgs & evtArgs) override {
            evtArgs.Handled(true);
            Input::PropagateOnMouseMove(evtArgs);
        }

        virtual void PropagateOnClick(MouseEventArgs & evtArgs) override {
            evtArgs.Handled(true);
            Input::PropagateOnClick(evtArgs);
        }

    public:
        virtual ~TextBox() = default;

        TextBox(const AllocType & allocator, Netcode::PxPtr<physx::PxRigidDynamic> pxActor) : Input{ allocator, std::move(pxActor) }, isPassword{ false } { }

        bool IsPassword() const {
            return isPassword;
        }

        void IsPassword(bool isPw) {
            isPassword = isPw;
        }

    };

}
