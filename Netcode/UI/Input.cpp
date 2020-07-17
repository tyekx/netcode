#include "Input.h"

namespace Netcode::UI {

    void Input::PropagateOnFocused(FocusChangedEventArgs & evtArgs) {

    }

    Input::Input(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor) : Label{ allocator, std::move(pxActor) }, tabIndex{ 0 }, focused{ false } {

    }

    int32_t Input::TabIndex() const {
        return tabIndex;
    }

    void Input::TabIndex(int32_t tbIndex) {
        tabIndex = tbIndex;
    }

    bool Input::Focused() const {
        return focused;
    }
}
