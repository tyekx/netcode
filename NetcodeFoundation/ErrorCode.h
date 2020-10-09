#pragma once

#include <system_error>

namespace Netcode {

	using ErrorCode = std::error_code;

	std::string ErrorCodeToString(const ErrorCode& ec);
	
}
