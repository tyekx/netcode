#pragma once

#include <NetcodeFoundation/ErrorCode.h>
#include <boost/beast/http.hpp>

namespace Netcode::Network {

	using ResponseBaseType = boost::beast::http::response<boost::beast::http::string_body>;

	class Response : public ResponseBaseType {
	protected:
		ErrorCode errorCode;
	public:
		using ResponseBaseType::ResponseBaseType;

		void SetErrorCode(const ErrorCode & ec) { errorCode = ec; }
		const ErrorCode & GetErrorCode() const { return errorCode; }

		Response(ResponseBaseType && rhs) noexcept : ResponseBaseType{ std::move(rhs) } { }
		Response(const ResponseBaseType & rhs) : ResponseBaseType{ rhs } { }
	};

}
