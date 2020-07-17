#pragma once

#include "Panel.h"

namespace Netcode::UI {

    class InputGroup : public Panel {
    protected:
    public:
        virtual ~InputGroup() = default;

        using Panel::Panel;
    };

}
