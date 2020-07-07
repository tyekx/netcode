#pragma once

#include <cassert>
#include <stdexcept>

namespace Netcode {

	using Exception = std::exception;

	class BaseException : public Exception {
		const char * s;
	public:
		BaseException(const char * s) : s{ s } {}

		virtual const char * what() const override {
			return s;
		}
	};

}
