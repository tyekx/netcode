#pragma once

#include "Label.h"

namespace Netcode::UI {

    class Input : public Label {
    protected:
        int32_t tabIndex;
        bool focused;

    public:
        virtual void PropagateOnFocused(FocusChangedEventArgs & evtArgs) override;
        virtual void PropagateOnBlurred(FocusChangedEventArgs & evtArgs) override;

        virtual ~Input() = default;

        Input(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor);

        int32_t TabIndex() const;

        void TabIndex(int32_t tbIndex);

        bool Focused() const;

    };

}
