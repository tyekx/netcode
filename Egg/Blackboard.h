#pragma once

#include "GameObject.h"

namespace Egg {

	class IBlackboard {
	public:
		virtual ~IBlackboard() = default;

		virtual void Handle(Egg::GameObject * self) = 0;
	};

}
