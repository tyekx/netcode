#pragma once

#include "../HandleTypes.h"
#include "../Event.hpp"
#include "../PhysXWrapper.h"
#include "../Graphics/GraphicsContexts.h"

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
		MouseEventArgs(const Netcode::Int2 & pos) : EventArgs{}, position{ pos } {

		}

		Netcode::Int2 Position() const {
			return position;
		}
	};

	class ScrollEventArgs : public MouseEventArgs {
		int32_t scrollVector;
	public:
		ScrollEventArgs(const Netcode::Int2 & pos, int32_t scrollVector) : MouseEventArgs{ pos }, scrollVector{ scrollVector } {

		}

		int32_t ScrollVector() const {
			return scrollVector;
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
