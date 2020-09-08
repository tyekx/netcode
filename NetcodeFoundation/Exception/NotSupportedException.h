#pragma once
#include "Common.h"

namespace Netcode {

	class NotSupportedException : public ExceptionBase {
	public:
		using ExceptionBase::ExceptionBase;

		virtual const char * what_type() const {
			return "NotSupportedException";
		}
	};

	class NotImplementedException : public ExceptionBase {
	public:
		using ExceptionBase::ExceptionBase;

		virtual const char * what_type() const {
			return "NotImplementedException";
		}
	};


}
