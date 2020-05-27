#pragma once
#include "Common.h"

namespace Netcode {

	class OutOfRangeException : public BaseException {
	public:
		using BaseException::BaseException;
	};

	class OutOfMemoryException : public BaseException {
	public:
		using BaseException::BaseException;
	};


}