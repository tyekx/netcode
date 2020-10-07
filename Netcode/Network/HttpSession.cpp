#include "HttpSession.h"
#include <Netcode/Logger.h>
#include "Response.hpp"
#include "CompletionToken.h"


namespace Netcode::Network {

	void HttpSession::OnReceive(ErrorCode ec, size_t transferredBytes, CompletionToken<Response> token) {
		if(ec) {
			Log::Error("[Network][Http] Failed to read from stream: {0}", ec.message());

			response.result(http::status::client_closed_request);

			token->Set(std::move(response));
		} else {
			Log::Debug("[Network][Http] Successfully read {0} bytes", static_cast<int32_t>(transferredBytes));

			token->Set(std::move(response));
		}
	}

	void HttpSession::OnSent(ErrorCode ec, size_t transferredBytes, CompletionToken<Response> token) {
		if(ec) {
			Log::Error("[Network][Http] Failed to write to stream: {0}", ec.message());

			response.result(http::status::client_closed_request);
			token->Set(std::move(response));
		} else {
			Log::Debug("[Network][Http] Successfully wrote {0} bytes", static_cast<int32_t>(transferredBytes));

			http::async_read(stream, readBuffer, response, [this, ct = std::move(token), lt = shared_from_this()](ErrorCode ec, size_t numBytes) mutable -> void {
				OnReceive(ec, numBytes, std::move(ct));
			});
		}
	}

	void HttpSession::OnConnected(ErrorCode ec, TcpEndpoint endpoint, CompletionToken<Response> token) {
		if(ec) {
			Log::Error("[Network][Http] Failed to connect to host: {0}", ec.message());

			response.result(http::status::client_closed_request);
			token->Set(std::move(response));
		} else {
			isConnected = true;

			boost::asio::ip::tcp::socket::keep_alive opt(true);
			stream.socket().set_option(opt, ec);
			if(ec) {
				Log::Error("[Network][Http] Failed to set socket option: {0}", ec.message());
			}

			Log::Debug("[Network][Http] Successfully connected to host");
			http::async_write(stream, request, [this, ct = std::move(token), lt = shared_from_this()](ErrorCode ec, size_t numBytes) mutable -> void {
				OnSent(ec, numBytes, std::move(ct));
			});
		}
	}

	void HttpSession::OnResolved(ErrorCode ec, boost::asio::ip::tcp::resolver::results_type results, CompletionToken<Response> token) {
		if(ec) {
			Log::Error("[Network][Http] Failed to resolve hostname: {0}", ec.message());
			response.result(http::status::client_closed_request);
			token->Set(std::move(response));
		} else {
			Log::Debug("[Network][Http] Successfully resolved hostname");
			stream.expires_after(std::chrono::seconds(30));

			stream.async_connect(results, [this, ct = std::move(token), lt = shared_from_this()](ErrorCode ec, TcpEndpoint endpoint) mutable -> void{
				OnConnected(ec, endpoint, std::move(ct));
			});
		}
	}

	void HttpSession::OnSentFirstTry(ErrorCode ec, size_t transferredBytes, std::string host, std::string port, CompletionToken<Response> token) {
		// if the stream "is just timed out", then try reconnecting first
		if(ec == boost::beast::error::timeout) {
			Log::Debug("[Network][Http] Stream timed out, trying to reconnect");
			stream.close();
			isConnected = false;
			resolver.async_resolve(host, port, [this, ct = std::move(token), lt = shared_from_this()](ErrorCode ec, TcpResolver::results_type results) mutable -> void {
				OnResolved(ec, std::move(results), std::move(ct));
			});
			return;
		}

		// on other errors, log for now
		if(ec) {
			Log::Error("[Network][Http] Failed to send HTTP request: {0}", ec.message());
			response.result(http::status::client_closed_request);
			token->Set(std::move(response));
			return;
		}

		http::async_read(stream, readBuffer, response, [this, ct = std::move(token), lt = shared_from_this()](ErrorCode ec, size_t numBytes) mutable -> void{
			OnReceive(ec, numBytes, std::move(ct));
		});
	}

	HttpSession::HttpSession(boost::asio::io_context & ioc) : ioc{ ioc }, strand { boost::asio::make_strand(ioc) }, resolver{ strand }, stream{ strand }, readBuffer{}, response{}, isConnected{ false } {

	}

	CompletionToken<Response> HttpSession::MakeRequest(std::string host, std::string port, std::string path, http::verb method, std::string cookies, std::string body) {
		CompletionToken<Response> token = std::make_shared<CompletionTokenType<Response>>(&ioc);
		

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
			resolver.async_resolve(host, port, [this, ct = token, lt = shared_from_this()](ErrorCode ec, TcpResolver::results_type results) mutable -> void {
				OnResolved(ec, std::move(results), std::move(ct));
			});
		} else {
			http::async_write(stream, request, [this, ct = token, h = std::move(host), p = std::move(port), lt = shared_from_this()]
				(ErrorCode ec, size_t numBytes) mutable -> void {
				OnSentFirstTry(ec, numBytes, std::move(h), std::move(p), std::move(ct));
			});
		}

		return token;
	}
}
