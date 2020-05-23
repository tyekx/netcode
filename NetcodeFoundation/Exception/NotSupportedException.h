#pragma once
#include "Common.h"

namespace Netcode {

	class NotSupportedException : public BaseException {
	public:
		using BaseException::BaseException;
	};

	class NotImplementedException : public BaseException {
	public:
		using BaseException::BaseException;
	};


}
