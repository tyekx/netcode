#pragma once

#include <boost/asio/ip/udp.hpp>

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

	template<typename T>
	class CompletionTokenType;

	class Response;

	template<typename T>
	using CompletionToken = std::shared_ptr<CompletionTokenType<T>>;

	using IpAddress = boost::asio::ip::address;
	using UdpSocket = boost::asio::ip::udp::socket;
	using UdpResolver = boost::asio::ip::udp::resolver;
	using UdpEndpoint = boost::asio::ip::udp::endpoint;

}
