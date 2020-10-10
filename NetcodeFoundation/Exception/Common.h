#pragma once

#include <cassert>
#include <stdexcept>

namespace Netcode {

	class ExceptionBase : public std::exception {
	public:
		ExceptionBase(const char * s) : std::exception{ s } {
			
		}

		// default format: {what_type()}: {what()}
		[[nodiscard]] virtual std::string ToString() const;
		
		[[nodiscard]] virtual const char * what_type() const {
			return "ExceptionBase";
		}
	};

}
