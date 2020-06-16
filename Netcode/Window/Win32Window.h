#pragma once

#include <NetcodeFoundation/Math.h>

namespace Netcode::Window {

	class IWindow {
	public:
		virtual void Show() = 0;
		virtual void Hide() = 0;
		virtual Int2 GetSize() = 0;
		virtual Int2 Resize(Int2 size) = 0;
	};

}
