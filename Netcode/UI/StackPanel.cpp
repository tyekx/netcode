#include "StackPanel.h"

namespace Netcode::UI {

    StackPanel::StackPanel(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor) : Panel{ allocator, std::move(pxActor) }, stackDirection{ Direction::VERTICAL } {

    }

    Direction StackPanel::StackDirection() const {
        return stackDirection;
    }

    void StackPanel::StackDirection(Direction dir) {
        stackDirection = dir;
    }

    void StackPanel::UpdateLayout() {
        Panel::UpdateLayout();

        const Vector2 mask = (StackDirection() == Direction::VERTICAL) ? Float2::UnitX : Float2::UnitY;
        const Vector2 invMask = mask.Swizzle<1, 0>();
        const Vector2 anchorOffset = mask * CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), Size());

        Vector2 dirSum = Float2::Zero;

        for(auto & child : children) {
            const Float2 bs = child->BoxSize();

            child->Position(invMask * anchorOffset + dirSum);

            dirSum += invMask * bs;
        }
    }

    Float2 StackPanel::DeriveSize() {
        Vector2 mask = (StackDirection() == Direction::VERTICAL) ? Float2::UnitY : Float2::UnitX;
        Vector2 invMask = mask.Swizzle<1, 0>();
        Vector2 derivedSize = Float2::Zero;

        for(auto & child : children) {
            Vector2 childSize = child->BoxSize();

            derivedSize = derivedSize.Max(invMask * childSize);
            derivedSize += mask * childSize;
        }

        return derivedSize;
    }

}
