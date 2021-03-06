#pragma once

#include "../HandleTypes.h"
#include "../Event.hpp"
#include "../Input.h"
#include "../Graphics/GraphicsContexts.h"

#include <Netcode/Input/Key.h>

namespace physx {

	class PxRigidDynamic;
	class PxPhysics;
	class PxScene;
	class PxMaterial;

}

namespace Netcode::UI {

	enum class Direction : uint32_t {
		VERTICAL, HORIZONTAL
	};

	enum class HorizontalAnchor : uint32_t {
		LEFT, CENTER, RIGHT
	};

	enum class VerticalAnchor : uint32_t {
		TOP, MIDDLE, BOTTOM
	};

	enum class SizingType : uint32_t {
		FIXED, DERIVED, INHERITED
	};

	enum class OverflowType : uint32_t {
		VISIBLE, HIDDEN
	};

	class Control;

	class EventArgs {
		Control * ctrl;
		bool handled;
	public:

		void Handled(bool isHandled) {
			handled = isHandled;
		}

		bool Handled() const {
			return handled;
		}

		Control * HandledBy() const {
			return ctrl;
		}

		void HandledBy(Control * pThis) {
			ctrl = pThis;
		}
	};

	class CharInputEventArgs : public EventArgs {
		wchar_t value;
	public:
		CharInputEventArgs(wchar_t value) : EventArgs{}, value{ value } { }

		wchar_t Value() const {
			return value;
		}
	};

	class MouseEventArgs : public EventArgs {
		Netcode::Int2 position;
		Netcode::Key key;
		Netcode::KeyModifiers modifier;

	public:
		MouseEventArgs(const Netcode::Int2 & pos, Netcode::Key key, Netcode::KeyModifiers modifier) : EventArgs{}, position{ pos }, key{ key }, modifier{ modifier } {

		}

		Netcode::Key Key() const {
			return key;
		}

		Netcode::KeyModifiers Modifier() const {
			return modifier;
		}

		Netcode::Int2 Position() const {
			return position;
		}
	};

	class DragEventArgs : public MouseEventArgs {
		Netcode::Int2 deltaPosition;
	public:
		DragEventArgs(const Netcode::Int2 & pos, Netcode::Key key, Netcode::KeyModifiers modifier, const Netcode::Int2 & deltaPos) :
			MouseEventArgs{ pos, key, modifier }, deltaPosition{ deltaPos } {

		}

		Netcode::Int2 DeltaPosition() const {
			return deltaPosition;
		}
	};

	class ScrollEventArgs : public MouseEventArgs {
		int32_t scrollVector;
	public:
		ScrollEventArgs(const Netcode::Int2 & pos, Netcode::Key key, Netcode::KeyModifiers modifier, int32_t scrollVector) : MouseEventArgs{ pos, key, modifier }, scrollVector{ scrollVector } {

		}

		int32_t ScrollVector() const {
			return scrollVector;
		}
	};

	class KeyEventArgs : public EventArgs {
		Netcode::Key key;
		Netcode::KeyModifiers modifier;
	public:
		KeyEventArgs(Netcode::Key key, Netcode::KeyModifiers modifier) : EventArgs{ }, key{ key }, modifier{ modifier } {

		}

		Netcode::KeyModifiers Modifier() const {
			return modifier;
		}

		Netcode::Key Key() const {
			return key;
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
