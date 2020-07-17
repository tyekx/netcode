#pragma once

#include "Input.h"

namespace Netcode::UI {


    class TextBox : public Input {
    protected:
        bool isPassword;

        virtual void PropagateOnMouseEnter(MouseEventArgs & evtArgs) override;
        virtual void PropagateOnMouseLeave(MouseEventArgs & evtArgs) override;
        virtual void PropagateOnMouseMove(MouseEventArgs & evtArgs) override;
        virtual void PropagateOnClick(MouseEventArgs & evtArgs) override;

    public:
        virtual ~TextBox() = default;

        TextBox(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor);
        bool IsPassword() const;
        void IsPassword(bool isPw);
    };

}
