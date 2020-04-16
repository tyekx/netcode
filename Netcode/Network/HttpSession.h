#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "../HandleTypes.h"
#include "../DestructiveCopyConstructible.hpp"

namespace Netcode::Network {

	namespace http = boost::beast::http;

	class HttpSession : public std::enable_shared_from_this<HttpSession> {
		using dcc_promise = dcc_t<std::promise<Netcode::Response>>;

		boost::asio::strand<boost::asio::io_context::executor_type> strand;
		boost::asio::ip::tcp::resolver resolver;
		boost::beast::tcp_stream stream;
		boost::beast::flat_buffer readBuffer;
		http::response<http::string_body> response;
		http::request<http::string_body> request;
		bool isConnected;

		void OnReceive(boost::system::error_code ec, std::size_t transferredBytes, dcc_promise promise);

		void OnSent(boost::system::error_code ec, std::size_t transferredBytes, dcc_promise promise);

		void OnConnected(boost::system::error_code ec, boost::asio::ip::tcp::endpoint endpoint, dcc_promise promise);

		void OnResolved(boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results, dcc_promise promise);

		void OnSentFirstTry(boost::system::error_code ec, std::size_t transferredBytes, std::string host, std::string port, dcc_promise promise);

	public:
		HttpSession(boost::asio::io_context & ioc);

		std::future<Netcode::Response> MakeRequest(std::string host, std::string port, std::string path, http::verb method,  std::string cookies, std::string body);
	};

	using HttpSessionRef = std::shared_ptr<HttpSession>;

}
