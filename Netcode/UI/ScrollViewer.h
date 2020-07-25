#pragma once

#include "Panel.h"

namespace Netcode::UI {

    class ScrollViewer : public Panel {
    protected:
        std::shared_ptr<Panel> scrollButton;
        Float2 maxSize;
        Float2 contentSize;
        Float4 scrollBarColor;
        float scrollY;
        float scrollBarThickness;

        void UpdateScrollButton();

        void ApplyScrollYDelta(float delta);

        virtual void Render(SpriteBatchPtr batch) override;

        virtual void PropagateOnSizeChanged() override;
    public:
        virtual ~ScrollViewer() = default;

        ScrollViewer(Memory::ObjectAllocator controlAllocator, const AllocType & eventAllocator, PxPtr<physx::PxRigidDynamic> actor);

        virtual void UpdateLayout() override;

        virtual void PropagateOnMouseScroll(ScrollEventArgs & args) override;

        virtual void PropagateOnDrag(DragEventArgs & args) override;

        virtual void AddChild(std::shared_ptr<Control> child) override;

        void ScrollButtonColor(const Float4 & color) {
            scrollButton->BackgroundColor(color);
        }

        Float4 ScrollButtonColor() const {
            return scrollButton->BackgroundColor();
        }

        void ScrollBarColor(const Float4 & color) {
            scrollBarColor = color;
        }

        Float4 ScrollBarColor() const {
            return scrollBarColor;
        }

        void ScrollBarThickness(float value) {
            scrollBarThickness = value;
        }

        float ScrollBarThickness() const {
            return scrollBarThickness;
        }

        void MaxSize(const Float2 & sz) {
            maxSize = sz;
        }

        Float2 MaxSize() const {
            return maxSize;
        }

        float ScrollY() const {
            return scrollY;
        }

    };

}