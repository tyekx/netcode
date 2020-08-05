#include "Panel.h"
#include <physx/PxRigidDynamic.h>
#include <Netcode/Graphics/SpriteDesc.h>
#include <Netcode/Graphics/BorderDesc.h>

namespace Netcode::UI {

    Netcode::BorderType Panel::BorderType() const {
        return borderType;
    }

    void Panel::BorderType(Netcode::BorderType type) {
        borderType = type;
    }

    Netcode::BackgroundType Panel::BackgroundType() const {
        return backgroundType;
    }

    void Panel::BackgroundType(Netcode::BackgroundType type) {
        backgroundType = type;
    }

    void Panel::BackgroundImageSize(const UInt2 & imgSize) {
        backgroundImageSize = imgSize;
    }

    SpriteDesc Panel::GetSpriteDesc() const {
        if(BackgroundType() == Netcode::BackgroundType::TEXTURE) {
            Float2 bgSize = BackgroundSize();

            if(bgSize.x == 0.0f || bgSize.y == 0.0f) {
                return SpriteDesc{ BackgroundImage(), BackgroundImageSize(), BackgroundColor() };
            } else {
                HorizontalAnchor ha = BackgroundHorizontalAlignment();
                VerticalAnchor va = BackgroundVerticalAlignment();

                Float2 controlSize = Size();

                Vector2 anchorDiff = CalculateAnchorOffset(ha, va, controlSize);
                Vector2 anchorOffset = CalculateAnchorOffset(ha, va, bgSize);

                Vector2 topLeftCorner = anchorOffset - anchorDiff;
                Vector2 bottomRightCorner = topLeftCorner + controlSize;

                Float2 tl = topLeftCorner;
                Float2 br = bottomRightCorner;

                Rect sourceRectangle{
                    static_cast<int32_t>(tl.x),
                    static_cast<int32_t>(tl.y),
                    static_cast<int32_t>(br.x),
                    static_cast<int32_t>(br.y)
                };

                return SpriteDesc{ BackgroundImage(), BackgroundImageSize(), sourceRectangle, BackgroundColor() };
            }
        }

        if(BackgroundType() == Netcode::BackgroundType::SOLID) {
            return SpriteDesc{ BackgroundColor() };
        }

        return SpriteDesc{ };
    }

    BorderDesc Panel::GetBorderDesc() const {
        if(BorderType() == Netcode::BorderType::SOLID) {
            return BorderDesc{ BorderWidth(), BorderRadius(), BorderColor() };
        }

        return BorderDesc{ };
    }

    Rect Panel::GetContentRect() const {
        Float2 pos = ScreenPosition();
        Float2 sz = Size();

        Rect rect;
        rect.left = static_cast<int32_t>(pos.x);
        rect.top = static_cast<int32_t>(pos.y);
        rect.right = static_cast<int32_t>(pos.x + sz.x);
        rect.bottom = static_cast<int32_t>(pos.y + sz.y);

        return rect;
    }

    UInt2 Panel::BackgroundImageSize() const {
        return backgroundImageSize;
    }

    Panel::Panel(const AllocType & alloc, PxPtr<physx::PxRigidDynamic> pxActor) : Control{ alloc, std::move(pxActor) }, borderType{ BorderType::NONE }, borderWidth{ 0.0f }, borderRadius{ 0.0f }, backgroundType{ BackgroundType::NONE }, backgroundVerticalAlignment{ VerticalAnchor::TOP }, backgroundHorizontalAlignment{ HorizontalAnchor::LEFT },
        backgroundImage{ nullptr }, backgroundSize{ Float2::Zero }, backgroundImageSize{ UInt2::Zero }, backgroundColor{ Float4::Zero }, borderColor{ Float4::Zero } { }

    VerticalAnchor Panel::BackgroundVerticalAlignment() const {
        return backgroundVerticalAlignment;
    }

    void Panel::BackgroundVerticalAlignment(VerticalAnchor verticalAlignment) {
        backgroundVerticalAlignment = verticalAlignment;
    }

    HorizontalAnchor Panel::BackgroundHorizontalAlignment() const {
        return backgroundHorizontalAlignment;
    }

    void Panel::BackgroundHorizontalAlignment(HorizontalAnchor horizontalAlignment) {
        backgroundHorizontalAlignment = horizontalAlignment;
    }

    Float4 Panel::BackgroundColor() const {
        return backgroundColor;
    }

    void Panel::ResetBackground() {
        BackgroundImage(nullptr);
        BackgroundColor(Float4::Zero);
        BackgroundSize(Float2::Zero);
        BackgroundType(BackgroundType::NONE);
    }

    void Panel::ResetBorder() {
        BorderRadius(0.0f);
        BorderWidth(0.0f);
        BorderColor(Float4::Zero);
        BorderType(Netcode::BorderType::NONE);
    }

    float Panel::BorderRadius() const {
        return borderRadius;
    }

    void Panel::BorderRadius(float br) {
        borderRadius = br;
        BorderType(Netcode::BorderType::SOLID);
    }

    void Panel::BorderWidth(float bw) {
        borderWidth = bw;
        BorderType(Netcode::BorderType::SOLID);
    }

    float Panel::BorderWidth() const {
        return borderWidth;
    }

    void Panel::BorderColor(const Float4 & c) {
        borderColor = c;
        BorderType(Netcode::BorderType::SOLID);
    }

    Float4 Panel::BorderColor() const {
        return borderColor;
    }

    Float2 Panel::BackgroundSize() const {
        return backgroundSize;
    }

    void Panel::BackgroundSize(const Float2 & bgSizeInPixels) {
        backgroundSize = bgSizeInPixels;
    }

    void Panel::BackgroundColor(const Float4 & color) {
        if(BackgroundType() == Netcode::BackgroundType::NONE) {
            BackgroundType(Netcode::BackgroundType::SOLID);
        }

        if(color.w == 0.0f) {
            BackgroundType(Netcode::BackgroundType::NONE);
        }

        backgroundColor = color;
    }

    float Panel::Opacity() const {
        return backgroundColor.w;
    }

    void Panel::Opacity(float w) {
        backgroundColor.w = w;
    }

    Ref<ResourceViews> Panel::BackgroundImage() const {
        return backgroundImage;
    }

    void Panel::BackgroundImage(std::nullptr_t) {
        BackgroundImage(nullptr, UInt2::Zero);
        BackgroundType(Netcode::BackgroundType::SOLID);
    }

    void Panel::BackgroundImage(Ref<ResourceViews> imageRef, const UInt2 & imageSize) {
        backgroundImage = imageRef;
        BackgroundImageSize(imageSize);
        if(imageRef != nullptr) {
            BackgroundType(Netcode::BackgroundType::TEXTURE);
        }
    }

    void Panel::Render(Ptr<SpriteBatch> batch) {
        SpriteDesc spriteDesc = GetSpriteDesc();
        BorderDesc borderDesc = GetBorderDesc();

        if(!spriteDesc.IsEmpty() || !borderDesc.IsEmpty()) {
            if(Overflow() == OverflowType::HIDDEN) {
                Rect rect = GetContentRect();

                int32_t rectWidth = rect.right - rect.left;
                int32_t rectHeight = rect.bottom - rect.top;

                int32_t rectArea = rectWidth * rectHeight;
                
                if(rectArea == 0) {
                    return;
                }

                batch->SetScissorRect(rect);
            }

            batch->DrawSprite(spriteDesc, borderDesc, ScreenPosition(), Size(), RotationOrigin(), RotationZ(), ZIndex());
        }

        Control::Render(batch);

        if(Overflow() == OverflowType::HIDDEN) {
            batch->SetScissorRect();
        }
    }

}
