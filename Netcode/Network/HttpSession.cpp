#include "HttpSession.h"
#include <Netcode/Logger.h>
#include <boost/bind.hpp>
#include "Response.hpp"


namespace Netcode::Network {

	void HttpSession::OnReceive(ErrorCode ec, std::size_t transferredBytes, dcc_promise promise) {
		if(ec) {
			Log::Error("[Network][Http] Failed to read from stream: {0}", ec.message());

			response.result(http::status::client_closed_request);
			promise.value.set_value(std::move(response));
		} else {
			Log::Debug("[Network][Http] Successfully read {0} bytes", static_cast<int32_t>(transferredBytes));
			promise.value.set_value(std::move(response));
		}
	}

	void HttpSession::OnSent(ErrorCode ec, std::size_t transferredBytes, dcc_promise promise) {
		if(ec) {
			Log::Error("[Network][Http] Failed to write to stream: {0}", ec.message());

			response.result(http::status::client_closed_request);
			promise.value.set_value(std::move(response));
		} else {
			Log::Debug("[Network][Http] Successfully wrote {0} bytes", static_cast<int32_t>(transferredBytes));

			http::async_read(stream, readBuffer, response,
				boost::bind(&HttpSession::OnReceive,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					promise));
		}
	}

	void HttpSession::OnConnected(ErrorCode ec, boost::asio::ip::tcp::endpoint endpoint, dcc_promise promise) {
		if(ec) {
			Log::Error("[Network][Http] Failed to connect to host: {0}", ec.message());

			response.result(http::status::client_closed_request);
			promise.value.set_value(std::move(response));
		} else {
			isConnected = true;

			boost::asio::ip::tcp::socket::keep_alive opt(true);
			stream.socket().set_option(opt, ec);
			if(ec) {
				Log::Error("[Network][Http] Failed to set socket option: {0}", ec.message());
			}

			Log::Debug("[Network][Http] Successfully connected to host");
			http::async_write(stream, request, boost::bind(&HttpSession::OnSent, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,
				promise));
		}
	}

	void HttpSession::OnResolved(ErrorCode ec, boost::asio::ip::tcp::resolver::results_type results, dcc_promise promise) {
		if(ec) {
			Log::Error("[Network][Http] Failed to resolve hostname: {0}", ec.message());

		} else {
			Log::Debug("[Network][Http] Successfully resolved hostname");
			stream.expires_after(std::chrono::seconds(30));

			stream.async_connect(results,
				boost::bind(&HttpSession::OnConnected, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::endpoint,
					promise));
		}
	}

	void HttpSession::OnSentFirstTry(ErrorCode ec, std::size_t transferredBytes, std::string host, std::string port, dcc_promise promise) {
		// if the stream "is just timed out", then try reconnecting first
		if(ec == boost::beast::error::timeout) {
			Log::Debug("[Network][Http] Stream timed out, trying to reconnect");
			stream.close();
			isConnected = false;
			resolver.async_resolve(host, port, boost::bind(&HttpSession::OnResolved, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::results,
				promise));
			return;
		}

		// on other errors, log for now
		if(ec) {
			Log::Error("[Network][Http] Failed to send HTTP request: {0}", ec.message());

			response.result(http::status::client_closed_request);
			promise.value.set_value(std::move(response));
			return;
		}

		http::async_read(stream, readBuffer, response,
			boost::bind(&HttpSession::OnReceive,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,
				promise));
	}

	HttpSession::HttpSession(boost::asio::io_context & ioc) : strand{ boost::asio::make_strand(ioc) }, resolver{ strand }, stream{ strand }, readBuffer{}, response{}, isConnected{ false } {

	}

	std::future<Response> HttpSession::MakeRequest(std::string host, std::string port, std::string path, http::verb method, std::string cookies, std::string body) {
		std::promise<Response> promise;
		auto future = promise.get_future();

		request.clear();
		request.method(method);
		request.target(path);
		request.version(11);
		request.set(http::field::host, host);
		request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		request.set(http::field::connection, "Keep-Alive");

		if(!cookies.empty()) {
			request.set(http::field::cookie, cookies);
		}

		if(!body.empty()) {
			request.set(http::field::content_type, "Application/JSON");
			request.set(http::field::content_length, body.size());
			request.body() = body;
		}

		if(!isConnected) {
			resolver.async_resolve(host, port, boost::bind(&HttpSession::OnResolved, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::results,
				move_to_dcc(promise)));
		} else {
			http::async_write(stream, request, boost::bind(&HttpSession::OnSentFirstTry, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,
				host,
				port,
				move_to_dcc(promise)));
		}

		return future;
	}
}
