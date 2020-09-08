#pragma once

#include "Common.h"

namespace Netcode {

	class UndefinedBehaviourException : public ExceptionBase {
	public:
		using ExceptionBase::ExceptionBase;

		virtual const char * what_type() const {
			return "UndefinedBehaviourException";
		}
	};

}

