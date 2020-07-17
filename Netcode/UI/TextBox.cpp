#include "TextBox.h"

namespace Netcode::UI {

	void TextBox::PropagateOnMouseEnter(MouseEventArgs & evtArgs) {
		evtArgs.Handled(true);
		Input::PropagateOnMouseEnter(evtArgs);
	}

	void TextBox::PropagateOnMouseLeave(MouseEventArgs & evtArgs) {
		evtArgs.Handled(true);
		Input::PropagateOnMouseLeave(evtArgs);
	}

	void TextBox::PropagateOnMouseMove(MouseEventArgs & evtArgs) {
		evtArgs.Handled(true);
		Input::PropagateOnMouseMove(evtArgs);
	}

	void TextBox::PropagateOnClick(MouseEventArgs & evtArgs) {
		evtArgs.Handled(true);
		Input::PropagateOnClick(evtArgs);
	}

	TextBox::TextBox(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor) : Input{ allocator, std::move(pxActor) }, isPassword{ false } { }

	bool TextBox::IsPassword() const {
		return isPassword;
	}

	void TextBox::IsPassword(bool isPw) {
		isPassword = isPw;
	}

}
