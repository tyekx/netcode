#pragma once

#include "Panel.h"

namespace Netcode::UI {

    class StackPanel : public Panel {
    protected:
        Direction stackDirection;

    public:

        virtual ~StackPanel() = default;

        StackPanel(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor);

        Direction StackDirection() const;

        void StackDirection(Direction dir);

        virtual void UpdateLayout() override;

        virtual Float2 DeriveSize() override;

    };

}