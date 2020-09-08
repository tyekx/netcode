#pragma once
#include "Common.h"

namespace Netcode {

	class IOException : public ExceptionBase {
		std::string filePath;
	public:

		IOException(const char * msg, const std::string & path = "") : ExceptionBase{ msg }, filePath{ path } {
			
		}

		// format: {what_type()}: {what()}. [Argument: '{GetFilePath()}']
		[[nodiscard]] virtual std::string ToString() const override;
		
		virtual const char * what_type() const override {
			return "IOException";
		}

		const std::string & GetFilePath() const {
			return filePath;
		}
	};

}
