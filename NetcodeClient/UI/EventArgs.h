#pragma once

#include <NetcodeFoundation/Math.h>
#include <Netcode/Input.h>

namespace UI {

	class EventArgs {
		bool handled;
	public:

		void Handled(bool isHandled) {
			handled = isHandled;
		}

		bool Handled() const {
			return handled;
		}
	};

	class MouseEventArgs : public EventArgs {
		Netcode::Int2 position;

	public:
		MouseEventArgs(const Netcode::Int2 & pos) : EventArgs{}, position { pos } {
		
		}

		Netcode::Int2 Position() const {
			return position;
		}
	};

	class KeyboardEventArgs : public EventArgs {
		uint32_t keys[8];
		uint32_t numKeys;
	public:
		Netcode::KeyModifiers modifiers;

		Netcode::ArrayView<uint32_t> Keys() const {
			return Netcode::ArrayView<uint32_t>(keys, numKeys);
		}
	};

	class FocusChangedEventArgs : public EventArgs {
		int32_t tabIndex;

	public:
		FocusChangedEventArgs(int32_t tabIndex) : EventArgs{}, tabIndex{ tabIndex } { }

		int32_t TabIndex() const {
			return tabIndex;
		}
	};

}
