#pragma once

#include "Common.h"
#include "../ErrorCode.h"

namespace Netcode {

	class RuntimeException : public ExceptionBase {
		ErrorCode errorCode;
		mutable std::string data;
	public:

		RuntimeException(const ErrorCode & ec) : ExceptionBase{ nullptr }, errorCode{ ec }, data{} {
			
		}

		// format: {what_type()}: {what()}. [Argument: '{GetFilePath()}']
		[[nodiscard]] virtual std::string ToString() const override {
			return ErrorCodeToString(errorCode);
		}
		
		[[nodiscard]]
		virtual const char * what_type() const override {
			return "RuntimeException";
		}
		
		[[nodiscard]] virtual const char * what() const override {
			if(data.empty()) {
				data = Netcode::ErrorCodeToString(errorCode);
			}
			return data.c_str();
		}
	};
	
}
