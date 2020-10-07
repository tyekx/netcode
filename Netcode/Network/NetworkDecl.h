#pragma once

namespace boost::system {

	class error_code;

}

namespace boost::asio {

	class io_context;
	class executor;

	namespace ip {

		class udp;
		class tcp;

	}

}

namespace std {

	template<typename T>
	class shared_ptr;

}

namespace Netcode::Network {

	class Timer;
	using ErrorCode = boost::system::error_code;

	template<typename T>
	class CompletionTokenType;

	class Response;

	template<typename T>
	using CompletionToken = std::shared_ptr<CompletionTokenType<T>>;

}
