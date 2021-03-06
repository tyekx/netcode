#include "Button.h"
#include <physx/PxRigidDynamic.h>
#include <physx/PxScene.h>

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

    void Button::PropagateOnMouseKeyPressed(MouseEventArgs & args) {
        args.Handled(true);
        Input::PropagateOnMouseKeyPressed(args);
    }

    void Button::PropagateOnMouseKeyReleased(MouseEventArgs & args) {
        args.Handled(true);
    	
        if(args.Key().GetCode() == KeyCode::MOUSE_LEFT) {
            MouseEventArgs copyArgs{ args.Position(), args.Key(), args.Modifier() };
            copyArgs.Handled(true);
            copyArgs.HandledBy(this);
            OnClick.Invoke(this, copyArgs);
        }

        Input::PropagateOnMouseKeyReleased(args);
    }

}
