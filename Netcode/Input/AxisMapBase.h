#pragma once

#include <Netcode/HandleDecl.h>

namespace Netcode {
	
	class AxisMapBase {
	public:
		virtual ~AxisMapBase() = default;
		virtual float GetAxis(uint32_t axisId) = 0;
		virtual void Update(ArrayView<Key> keys) = 0;
	};

}
