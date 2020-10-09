#include "ErrorCode.h"
#include <sstream>

namespace Netcode {

	std::string ErrorCodeToString(const ErrorCode & ec)
	{
		std::ostringstream oss;
		if(ec) {
			oss << "Error(" << ec.category().name() << "#" << ec.value() << "): " << ec.message();
		} else {
			oss << "Success(" << ec.category().name() << ")";
		}
		return oss.str();
	}

}
