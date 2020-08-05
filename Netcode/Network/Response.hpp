#pragma once

namespace Netcode::Network {

	using ResponseBaseType = boost::beast::http::response<boost::beast::http::string_body>;

	class Response : public ResponseBaseType {
	public:
		using ResponseBaseType::ResponseBaseType;

		Response(ResponseBaseType && rhs) noexcept : ResponseBaseType{ std::move(rhs) } { }
		Response(const ResponseBaseType & rhs) : ResponseBaseType{ rhs } { }
	};

}
