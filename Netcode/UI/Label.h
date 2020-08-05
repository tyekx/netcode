#pragma once

#include "Panel.h"

namespace Netcode::UI {

    class Label : public Panel {
    protected:
        Ref<SpriteFont> font;
        Float4 textColor;
        std::wstring text;
        Float2 textPosition;

        void TextPosition(const Float2 & tp);

        Float2 TextPosition() const;

        void UpdateTextPosition();

        void UpdateTextSize();

        virtual void PropagateOnFontChanged();

        virtual void PropagateOnTextChanged();

    public:
        virtual ~Label() = default;

        Label(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor);

        void Font(Ref<SpriteFont> ref);
        Ref<SpriteFont> Font() const;

        Float4 TextColor() const;

        void TextColor(const Float4 & color);

        Float3 TextRGB() const;

        void TextRGB(const Float3 & rgb);

        float TextOpacity() const;

        void TextOpacity(float w);

        const std::wstring & Text() const;

        void Text(const std::wstring & ws);

        virtual void Render(Ptr<SpriteBatch> batch) override;
    };

}
