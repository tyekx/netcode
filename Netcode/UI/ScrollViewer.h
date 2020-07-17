#pragma once

#include "Panel.h"

namespace Netcode::UI {

    class ScrollViewer : public Panel {
    protected:
    public:
        virtual ~ScrollViewer() = default;

        using Panel::Panel;
    };

}