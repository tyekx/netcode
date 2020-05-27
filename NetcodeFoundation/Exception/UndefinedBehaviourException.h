#pragma once

#include "Common.h"

namespace Netcode {

	class UndefinedBehaviourException : public BaseException {
	public:
		using BaseException::BaseException;
	};

}

