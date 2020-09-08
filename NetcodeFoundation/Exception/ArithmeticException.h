#pragma once
#include "Common.h"

namespace Netcode {

	class ArithmeticException : public ExceptionBase {
	public:
		using ExceptionBase::ExceptionBase;

		virtual const char * what_type() const {
			return "ArithmeticException";
		}
	};

}
