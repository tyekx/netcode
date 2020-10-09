#pragma once

#include "Input.h"

namespace Netcode::UI {

    class Button : public Input {
    protected:
    public:
        virtual void PropagateOnClick(MouseEventArgs& args) override {
            return;
        }
    	
        virtual void PropagateOnMouseEnter(MouseEventArgs & evtArgs) override;

        virtual void PropagateOnMouseLeave(MouseEventArgs & evtArgs) override;

        virtual void PropagateOnMouseMove(MouseEventArgs & evtArgs) override;

        virtual void PropagateOnMouseKeyPressed(MouseEventArgs & args) override;

        virtual void PropagateOnMouseKeyReleased(MouseEventArgs & args) override;

        virtual ~Button() = default;

        using Input::Input;
    };

}
