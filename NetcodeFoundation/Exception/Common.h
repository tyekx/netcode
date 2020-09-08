#pragma once

#include <cassert>
#include <stdexcept>

namespace Netcode {

	class ExceptionBase : public std::exception {
	protected:
		const char * message;
	public:
		ExceptionBase(const char * s) : message{ s } {}

		// default format: {what_type()}: {what()}
		[[nodiscard]] virtual std::string ToString() const;
		
		[[nodiscard]] virtual const char * what_type() const {
			return "ExceptionBase";
		}
		
		[[nodiscard]] virtual const char * what() const override {
			return message;
		}
	};

}
