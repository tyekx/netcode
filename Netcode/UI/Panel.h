#pragma once

#include "Control.h"

namespace Netcode::UI {

    class Panel : public Control {
    protected:
        BorderType borderType;
        float borderWidth;
        float borderRadius;
        BackgroundType backgroundType;
        VerticalAnchor backgroundVerticalAlignment;
        HorizontalAnchor backgroundHorizontalAlignment;
        Ref<ResourceViews> backgroundImage;
        Float2 backgroundSize;
        UInt2 backgroundImageSize;
        Float4 backgroundColor;
        Float4 borderColor;

        Netcode::BorderType BorderType() const;

        void BorderType(Netcode::BorderType type);

        Netcode::BackgroundType BackgroundType() const;

        void BackgroundType(Netcode::BackgroundType type);

        void BackgroundImageSize(const UInt2 & imgSize);

        SpriteDesc GetSpriteDesc() const;

        BorderDesc GetBorderDesc() const;

        Rect GetContentRect() const;

    public:
        UInt2 BackgroundImageSize() const;

        Panel(const AllocType & alloc, PxPtr<physx::PxRigidDynamic> pxActor);

        virtual ~Panel() = default;

        VerticalAnchor BackgroundVerticalAlignment() const;

        void BackgroundVerticalAlignment(VerticalAnchor verticalAlignment);

        HorizontalAnchor BackgroundHorizontalAlignment() const;

        void BackgroundHorizontalAlignment(HorizontalAnchor horizontalAlignment);

        Float4 BackgroundColor() const;

        void ResetBackground();

        void ResetBorder();

        float BorderRadius() const;

        void BorderRadius(float br);

        void BorderWidth(float bw);

        float BorderWidth() const;

        void BorderColor(const Float4 & c);

        Float4 BorderColor() const;

        void BorderOpacity(float v);

        float BorderOpacity() const;

        void BorderRGB(const Float3 & c);

        Float3 BorderRGB() const;

        Float2 BackgroundSize() const;

        void BackgroundSize(const Float2 & bgSizeInPixels);

        void BackgroundColor(const Float4 & color);

        float Opacity() const;

        void Opacity(float w);

        Ref<ResourceViews> BackgroundImage() const;

        void BackgroundImage(std::nullptr_t);

        void BackgroundImage(Ref<ResourceViews> imageRef, const UInt2 & imageSize);

        virtual void Render(Ptr<SpriteBatch> batch) override;
    };

}