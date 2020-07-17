#pragma once

#include "Input.h"

namespace Netcode::UI {

    class Button : public Input {
    protected:
        virtual void PropagateOnMouseEnter(MouseEventArgs & evtArgs) override;

        virtual void PropagateOnMouseLeave(MouseEventArgs & evtArgs) override;

        virtual void PropagateOnMouseMove(MouseEventArgs & evtArgs) override;

        virtual void PropagateOnClick(MouseEventArgs & evtArgs) override;

    public:
        virtual ~Button() = default;

        using Input::Input;
    };

}
