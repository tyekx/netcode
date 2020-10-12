#pragma once

#include <NetcodeFoundation/ErrorCode.h>

namespace Netcode {

	enum class NetworkErrc {

		SUCCESS,

		// lower layers
		HOSTNAME_NOT_FOUND,
		NO_NETWORK_INTERFACE,
		SOCK_ERROR,

		// message parsing related issues
		MISSING_HEADER,
		MISSING_FRAGMENT_DATA,
		HEADER_TOO_BIG,
		MESSAGE_TOO_BIG,
		INVALID_MTU,
		INCONSISTENT_FRAGMENTS,
		BAD_HEADER,
		BAD_MESSAGE,

		// protocol semantic related issues
		BAD_REQUEST,
		RESEND_TIMEOUT,
		RESPONSE_TIMEOUT,
		ALREADY_CONNECTED,
		SERVER_FULL,
		UNAUTHORIZED,

		// gameplay related related issues
		CLIENT_KICKED,
		CLIENT_BANNED,
		CLIENT_TIMEOUT,
		SERVER_TIMEOUT,
		SERVER_CLOSED,

		// http
		BAD_COOKIE
		
	};

	class NetworkErrorCategory : public std::error_category {
	public:
		[[nodiscard]]
		const char * name() const noexcept override
		{
			return "Netcode.Net";
		}

		[[nodiscard]]
		std::string message(int v) const override
		{
			switch(static_cast<NetworkErrc>(v)) {
				case NetworkErrc::SUCCESS: return "Success";
				case NetworkErrc::HOSTNAME_NOT_FOUND: return "Host not found";
				case NetworkErrc::NO_NETWORK_INTERFACE: return "No network interface";
				case NetworkErrc::SOCK_ERROR: return "Socket error";
				case NetworkErrc::MISSING_HEADER: return "Missing header";
				case NetworkErrc::MISSING_FRAGMENT_DATA: return "Missing fragment data";
				case NetworkErrc::HEADER_TOO_BIG: return "Header too big";
				case NetworkErrc::MESSAGE_TOO_BIG: return "Message too big";
				case NetworkErrc::INVALID_MTU: return "MTU value is invalid";
				case NetworkErrc::INCONSISTENT_FRAGMENTS: return "Fragments are inconsistent";
				case NetworkErrc::BAD_HEADER: return "Failed to parse header";
				case NetworkErrc::BAD_MESSAGE: return "Failed to parse message";
				case NetworkErrc::BAD_REQUEST: return "Bad request";
				case NetworkErrc::RESEND_TIMEOUT: return "Send operation timed out";
				case NetworkErrc::RESPONSE_TIMEOUT: return "Receive operation timed out";
				case NetworkErrc::ALREADY_CONNECTED: return "Client already connected";
				case NetworkErrc::SERVER_FULL: return "Server is full";
				case NetworkErrc::UNAUTHORIZED: return "Unauthorized operation";
				case NetworkErrc::CLIENT_KICKED: return "Client kicked";
				case NetworkErrc::CLIENT_BANNED: return "Client banned";
				case NetworkErrc::CLIENT_TIMEOUT: return "Client timed out";
				case NetworkErrc::SERVER_TIMEOUT: return "Server timed out";
				case NetworkErrc::SERVER_CLOSED: return "Server closed";
				case NetworkErrc::BAD_COOKIE: return "Cookie was not set";
			}

			return "Unknown network error";
		}
	};

	inline ErrorCode make_error_code(NetworkErrc e) {
		static NetworkErrorCategory cat;
		return ErrorCode{ static_cast<int>(e), cat };
	}
	
}

namespace std {
	template<>
	struct is_error_code_enum<Netcode::NetworkErrc> {
		static const bool value = true;
	};
}
