#pragma once
#include "Common.h"

namespace Netcode {

	class OutOfRangeException : public ExceptionBase {
	public:
		using ExceptionBase::ExceptionBase;

		virtual const char * what_type() const {
			return "OutOfRangeException";
		}
	};

	class OutOfMemoryException : public ExceptionBase {
	public:
		using ExceptionBase::ExceptionBase;

		virtual const char * what_type() const {
			return "OutOfMemoryException";
		}
	};


}