#include "Input.h"

namespace Netcode::UI {

    void Input::PropagateOnFocused(FocusChangedEventArgs & evtArgs) {
        if(evtArgs.TabIndex() == TabIndex()) {
            focused = true;
            evtArgs.Handled(true);
        }
        Label::PropagateOnFocused(evtArgs);
    }

    void Input::PropagateOnBlurred(FocusChangedEventArgs & evtArgs)
    {
        focused = false;
        evtArgs.Handled(true);
        Label::PropagateOnBlurred(evtArgs);
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
