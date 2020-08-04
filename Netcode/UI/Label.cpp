#include "Label.h"
#include <physx/PxRigidDynamic.h>
#include <physx/PxScene.h>

namespace Netcode::UI {

    void Label::PropagateOnFontChanged() {
        if(font != nullptr) {
            UpdateTextSize();
        }
    }

    void Label::PropagateOnTextChanged() {
        if(font != nullptr) {
            UpdateTextSize();
        }
    }

    void Label::Font(SpriteFontRef ref) {
        if(font != ref) {
            font = ref;
            PropagateOnFontChanged();
        }
    }

    SpriteFontRef Label::Font() const {
        return font;
    }

    void Label::TextPosition(const Float2 & pos) {
        textPosition = pos;
    }

    Float2 Label::TextPosition() const {
        return textPosition;
    }

    void Label::UpdateTextPosition() {
        const Float2 textSize = font->MeasureString(text.c_str());
        const Vector2 textAnchorOffset = CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), Size());
        const Vector2 textAnchorDiff = CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), textSize);
        TextPosition(textAnchorOffset - textAnchorDiff);
    }

    void Label::UpdateTextSize() {
        if(Sizing() == SizingType::DERIVED) {
            Size(font->MeasureString(text.c_str()));
            TextPosition(Float2::Zero);
        } else {
            if(text.empty()) {
                TextPosition(Float2::Zero);
            } else {
                UpdateTextPosition();
            }
        }
    }

    Label::Label(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor) : Panel{ allocator, std::move(pxActor) }, font{ nullptr }, textColor{ Float4::Zero }, text{}, textPosition{ Float2::Zero } {

    }

    Float4 Label::TextColor() const {
        return textColor;
    }

    void Label::TextColor(const Float4 & color) {
        textColor = color;
    }

    Float3 Label::TextRGB() const {
        return Float3{ textColor.x, textColor.y, textColor.z };
    }

    void Label::TextRGB(const Float3 & rgb) {
        TextColor(Float4{ rgb.x, rgb.y, rgb.z, TextColor().w });
    }

    float Label::TextOpacity() const {
        return textColor.w;
    }

    void Label::TextOpacity(float w) {
        textColor.w = w;
    }

    const std::wstring & Label::Text() const {
        return text;
    }

    void Label::Text(const std::wstring & ws) {
        if(text != ws) {
            text = ws;
            PropagateOnTextChanged();
        }
    }

    void Label::Render(SpriteBatchPtr batch)  {
        Panel::Render(batch);

        if(font != nullptr && !text.empty()) {
            const Vector2 screenPos = ScreenPosition();
            const Vector2 textPos = TextPosition();

            font->DrawString(batch, text, screenPos + textPos, TextColor(), RotationOrigin(), RotationZ(), ZIndex() + 0.1f);
        }
    }

}
