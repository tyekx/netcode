#pragma once

#include "NetworkDecl.h"
#include <Netcode/HandleDecl.h>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace Netcode::Network {

	namespace http = boost::beast::http;

	using TcpResolver = boost::asio::ip::tcp::resolver;
	using TcpEndpoint = boost::asio::ip::tcp::endpoint;

	class HttpSession : public std::enable_shared_from_this<HttpSession> {

		boost::asio::io_context & ioc;
		boost::asio::strand<boost::asio::io_context::executor_type> strand;
		TcpResolver resolver;
		boost::beast::tcp_stream stream;
		boost::beast::flat_buffer readBuffer;
		http::response<http::string_body> response;
		http::request<http::string_body> request;
		bool isConnected;

		void OnReceive(ErrorCode ec, size_t transferredBytes, CompletionToken<Response> token);

		void OnSent(ErrorCode ec, size_t transferredBytes, CompletionToken<Response> token);

		void OnConnected(ErrorCode ec, TcpEndpoint endpoint, CompletionToken<Response> token);

		void OnResolved(ErrorCode ec, TcpResolver::results_type results, CompletionToken<Response> token);

		void OnSentFirstTry(ErrorCode ec, size_t transferredBytes, std::string host, std::string port, CompletionToken<Response> token);

	public:
		HttpSession(boost::asio::io_context & ioc);

		CompletionToken<Response> MakeRequest(std::string host, std::string port, std::string path, http::verb method,  std::string cookies, std::string body);
	};

}
