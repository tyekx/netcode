#pragma once

#include "Input.h"

namespace Netcode::UI {


    class TextBox : public Input {
    protected:
        static std::weak_ptr<Panel> globalCaret;
        Float4 selectionTextColor;
        Float4 selectionBackgroundColor;
        Float4 caretColor;
        float xSlider;
        int32_t caretPosition;
        int32_t selectionOffset;
        bool isPassword;
        std::shared_ptr<Panel> caretRef;

        static std::shared_ptr<Panel> CreateCaret(const Memory::ObjectAllocator & alloc);

        std::wstring_view GetView() const;
        int32_t GetTargetedCaretPosition(float xWindowPos);

        virtual void PropagateOnMouseEnter(MouseEventArgs & evtArgs) override;
        virtual void PropagateOnMouseLeave(MouseEventArgs & evtArgs) override;
        virtual void PropagateOnMouseMove(MouseEventArgs & evtArgs) override;
        virtual void PropagateOnClick(MouseEventArgs & evtArgs) override;
        virtual void PropagateOnKeyPressed(KeyEventArgs & args) override;
        virtual void PropagateOnCharInput(CharInputEventArgs & args) override;
        virtual void PropagateOnBlurred(FocusChangedEventArgs & args) override;

        virtual void Render(SpriteBatchPtr batch) override;

        void EraseSelection();

    public:
        virtual ~TextBox() = default;

        void AppendChar(wchar_t c);

        void HandleDefaultKeyStrokes(Key key, KeyModifier modifier);

        virtual void Update(float dt) override;

        TextBox(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor);

        void SelectionTextColor(const Float4 & color);

        Float4 SelectionTextColor() const;

        void SelectionBackgroundColor(const Float4 & color);

        Float4 SelectionBackgroundColor() const;

        void CaretColor(const Float4 & color);

        Float4 CaretColor() const;

        bool IsPassword() const;
        void IsPassword(bool isPw);
    };

}
