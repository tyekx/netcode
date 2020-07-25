#include "ScrollViewer.h"

namespace Netcode::UI {

    namespace Detail {

        /**
        * Private implementation just to override a few handlers
        * this class indicates that the event is handled, but still forward it to its parent
        */
        class ScrollButton : public Panel {
        public:
            using Panel::Panel;

            virtual void PropagateOnPositionChanged() override {
                screenPositionCache = CalculateScreenPosition();
                UpdateActorPose();
            }

            virtual void PropagateOnSizeChanged() override {
                UpdateActorShape();
            }

            virtual void PropagateOnDrag(DragEventArgs & args) override {
                args.Handled(true);
                args.HandledBy(this);
                if(parent != nullptr) {
                    parent->PropagateOnDrag(args);
                }
            }
        };

    }

    void ScrollViewer::UpdateScrollButton()
    {
        if(children.empty()) {
            return;
        }

        std::shared_ptr<Control> ctrl = children.front();

        contentSize = ctrl->BoxSize();

        Vector2 cSize = contentSize;
        Vector2 mSize = MaxSize();

        size = mSize.Min(cSize);

        if((contentSize.y - 0.5f) < maxSize.y) {
            scrollButton->Enabled(false);
        } else {
            scrollButton->Enabled(true);

            Float2 btnSize = (mSize / cSize) * mSize;

            scrollButton->Size(Float2{ ScrollBarThickness(), btnSize.y });
        }
    }

    void ScrollViewer::ApplyScrollYDelta(float delta)
    {
        float maxValue = std::max(contentSize.y - maxSize.y, 0.0f);
        scrollY = std::clamp(scrollY + delta, 0.0f, maxValue);

        if(!children.empty()) {
            children.front()->Position(Float2{ 0.0f, -scrollY });
        }
    }

    void ScrollViewer::Render(SpriteBatchPtr batch)
    {
        if(children.empty()) {
            return;
        }

        Float2 pos = ScreenPosition();
        Vector2 posV = pos;
        Vector2 sz = Size();
        Float2 rb = sz + pos;

        Rect rect;
        rect.left = static_cast<int32_t>(pos.x);
        rect.top = static_cast<int32_t>(pos.y);
        rect.right = static_cast<int32_t>(rb.x);
        rect.bottom = static_cast<int32_t>(rb.y);

        Vector2 yOffset = Float2{ 0.0f, scrollY };
        Vector2 cSize = contentSize;
        Vector2 mSize = MaxSize();
        Vector2 scaledYOffset = yOffset * (mSize / cSize);

        batch->SetScissorRect(rect);
        children.front()->Render(batch);

        if(scrollButton->Enabled()) {
            Float2 scrollBarSize{ ScrollBarThickness(), Size().y };

            Vector2 anchorOffset = CalculateAnchorOffset(HorizontalAnchor::RIGHT, VerticalAnchor::TOP, Size());
            Vector2 anchorDiff = CalculateAnchorOffset(HorizontalAnchor::RIGHT, VerticalAnchor::TOP, scrollBarSize);

            Vector2 alignment = anchorOffset - anchorDiff;

            batch->DrawSprite(SpriteDesc{ ScrollBarColor() }, posV + alignment, scrollBarSize);

            scrollButton->Position(alignment + scaledYOffset);
            scrollButton->Render(batch);
        }
        batch->SetScissorRect();
    }

    void ScrollViewer::PropagateOnSizeChanged()
    {
        UpdateScrollButton();

        Panel::PropagateOnSizeChanged();
    }

    ScrollViewer::ScrollViewer(Memory::ObjectAllocator controlAllocator, const AllocType & eventAllocator, PxPtr<physx::PxRigidDynamic> actor) :
        Panel{ eventAllocator, std::move(actor) },
        scrollButton{},
        maxSize{},
        contentSize{},
        scrollBarColor{ Float4::Zero },
        scrollY{},
        scrollBarThickness{ 20.0f } {

        physx::PxScene * scene = pxActor->getScene();

        UndefinedBehaviourAssertion(scene != nullptr);
        UndefinedBehaviourAssertion(pxActor->getNbShapes() == 1);

        physx::PxShape * shape;
        pxActor->getShapes(&shape, 1);

        UndefinedBehaviourAssertion(shape->getNbMaterials() == 1);

        physx::PxMaterial * mat;
        shape->getMaterials(&mat, 1);

        PxPtr<physx::PxRigidDynamic> clonedActor = physx::PxCreateDynamic(scene->getPhysics(), physx::PxTransform{ physx::PxIdentity }, physx::PxBoxGeometry{ 1.0f, 1.0f, 1.0f }, * mat, 0.5f);
        scene->addActor(*clonedActor);

        scrollButton = controlAllocator.MakeShared<Detail::ScrollButton>(eventAllocator, std::move(clonedActor));
        scrollButton->Sizing(SizingType::FIXED);
        scrollButton->Size(Float2{ 40.0f, 40.0f });
        scrollButton->BackgroundColor(Float4::One);
        scrollButton->ZIndex(128.0f);
        scrollButton->Enabled(false);
    }

    void ScrollViewer::UpdateLayout()
    {
        UndefinedBehaviourAssertion(Sizing() != SizingType::INHERITED);

        for(auto & child : children) {
            child->UpdateLayout();
        }

        screenPositionCache = CalculateScreenPosition();

        if(!children.empty()) {
            // bypass any user setting with these
            HorizontalContentAlignment(HorizontalAnchor::LEFT);
            VerticalContentAlignment(VerticalAnchor::TOP);
            children.front()->Position(Float2::Zero);
        }

        scrollY = 0.0f;

        UpdateScrollButton();
    }

    void ScrollViewer::PropagateOnMouseScroll(ScrollEventArgs & args)
    {
        ApplyScrollYDelta(static_cast<float>(-args.ScrollVector() / 8));

        args.Handled(true);

        Panel::PropagateOnMouseScroll(args);
    }

    void ScrollViewer::PropagateOnDrag(DragEventArgs & args) {
        if(args.Handled()) {
            Float2 deltaPos{ static_cast<float>(args.DeltaPosition().x), static_cast<float>(args.DeltaPosition().y) };

            if(deltaPos.y != 0) {
                float scrollScale = (contentSize.y - maxSize.y) / (maxSize.y - scrollButton->Size().y);
                
                ApplyScrollYDelta(static_cast<float>(deltaPos.y) * scrollScale);
            }
        }
    }

    void ScrollViewer::AddChild(std::shared_ptr<Control> child)
    {
        if(child != nullptr) {
            children.clear();
            Panel::AddChild(std::move(child));
            Panel::AddChild(scrollButton);
        }
    }

}
