#include "Button.h"

namespace Netcode::UI {

    void Button::PropagateOnMouseEnter(MouseEventArgs & evtArgs) {
        evtArgs.Handled(true);
        Input::PropagateOnMouseEnter(evtArgs);
    }

    void Button::PropagateOnMouseLeave(MouseEventArgs & evtArgs) {
        evtArgs.Handled(true);
        Input::PropagateOnMouseLeave(evtArgs);
    }

    void Button::PropagateOnMouseMove(MouseEventArgs & evtArgs) {
        evtArgs.Handled(true);
        Input::PropagateOnMouseMove(evtArgs);
    }

    void Button::PropagateOnClick(MouseEventArgs & evtArgs) {
        evtArgs.Handled(true);
        Input::PropagateOnClick(evtArgs);
    }

}
