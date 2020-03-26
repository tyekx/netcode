#include "NetcodeNetworkModule.h"
#include <boost/range.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <memory>
#include <functional>
#include "Egg/Utility.h"
#include "Egg/DestructiveCopyConstructible.hpp"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
constexpr static int EXPIRES_IN = 30;

void HandleCommunication(
	std::string host,
	std::string port,
	http::verb method,
	std::string path,
	std::string cookies,
	std::string body,
	Egg::dcc_t<std::promise<Egg::Response>> promise,
	Egg::Module::CookieStorage& cookieStorage,
	boost::asio::yield_context yield) {
	boost::system::error_code ec;

	tcp::resolver resolver{ IOEngine::GetIOContext() };
	boost::beast::tcp_stream stream{ IOEngine::GetIOContext() };
	http::request<http::string_body> request{ method, path, 11 };
	request.set(http::field::host, host);
	request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
	request.set(http::field::connection, "Closed");

	if(!cookies.empty()) {
		request.set(http::field::cookie, cookies);
	}

	if(!body.empty()) {
		request.set(http::field::content_type, "Application/JSON");
		request.set(http::field::content_length, body.size());
		request.body() = body;
	}

	const auto results = resolver.async_resolve(host, port, yield[ec]);

	if(ec) {
		Log::Error("Failed to resolve hostname");
		promise.value.set_value(Egg::Response(static_cast<int>(http::status::service_unavailable)));
		return;
	}

	stream.expires_after(std::chrono::seconds(EXPIRES_IN));

	stream.async_connect(results, yield[ec]);

	if(ec) {
		Log::Error("Failed to establish connection");
		Log::Error(ec.message().c_str());
		return;
	}

	size_t writtenSize = http::async_write(stream, request, yield[ec]);

	if(ec) {
		Log::Error("Failed to write to stream");
		Log::Error(ec.message().c_str());
		promise.value.set_value(Egg::Response(static_cast<int>(http::status::service_unavailable)));
		return;
	}

	boost::beast::flat_buffer responseBuffer;

	http::response<http::string_body> response;

	size_t readSize = http::async_read(stream, responseBuffer, response, yield[ec]);

	if(ec) {
		Log::Error("Failed to receive from server");
		promise.value.set_value(Egg::Response(static_cast<int>(http::status::service_unavailable)));
		return;
	}

	auto it = response.find(http::field::set_cookie);

	if(it != response.end()) {
		size_t s = response.count(http::field::set_cookie);

		std::vector<Cookie> newCookies;

		for(size_t i = 0; i < s; ++i) {
			Cookie c;

			boost::beast::string_view sv = it->value();

			if(Cookie::Parse(std::string_view(sv.data(), sv.size()), c)) {
				newCookies.push_back(c);
			}

			++it;
		}

		cookieStorage.SetCookies(std::move(newCookies));
	}

	promise.value.set_value(Egg::Response(response.result_int(), response.body()));

	stream.socket().shutdown(tcp::socket::shutdown_both, ec);

	if(ec && ec != boost::beast::errc::not_connected) {
		Log::Error("Failed to shut down socket properly");
		return;
	}

	Log::Debug("Gracefully closed");

}

std::future<Egg::Response> MakeRequest(std::string host, std::string port, http::verb verb, std::string path, std::string cookies, std::string body, Egg::Module::CookieStorage& cookieStorage) {
	std::promise<Egg::Response> promise = std::promise<Egg::Response>();
	auto rv = promise.get_future();

	boost::asio::spawn(IOEngine::GetIOContext(),
		std::bind(&HandleCommunication,
			host, port, verb, path, cookies, body, Egg::move_to_dcc(promise), std::ref( cookieStorage ), std::placeholders::_1));

	return rv;
}

namespace Egg::Module {

	std::string CookieStorage::GetCookies() const
	{
		std::lock_guard<std::mutex> guard{ mutex };

		return cookiesCache;
	}

	void CookieStorage::SetCookies(std::vector<Cookie> cks)
	{
		std::lock_guard<std::mutex> guard{ mutex };

		for(const Cookie & c : cks) {
			cookies[c.GetName()] = c;
		}

		cookiesCache.clear();

		bool isFirst = true;

		for(const auto & i : cookies) {
			if(!isFirst) {
				cookiesCache += "; ";
			} else isFirst = false;
			cookiesCache += i.second.GetCookieString();
		}
	}

	Cookie CookieStorage::GetCookie(const std::string & key) const
	{
		std::lock_guard<std::mutex> guard{ mutex };

		auto it = cookies.find(key);

		if(it != cookies.end()) {
			return it->second;
		}

		return Cookie();
	}

	void NetcodeNetworkModule::Start(AApp * app) {
		IOEngine::Init(1);
	}

	void NetcodeNetworkModule::Shutdown() {
		IOEngine::Stop();
	}

	void NetcodeNetworkModule::SetWebserverAddress(std::string ipAddr, int port) {
		webserverAddr = std::move(ipAddr);
		webserverPort = port;
	}

	Cookie NetcodeNetworkModule::GetCookie(const std::string & key)
	{
		return cookieStorage.GetCookie(key);
	}

	std::future<Response> NetcodeNetworkModule::QueryServers() {
		std::string cookies = cookieStorage.GetCookies();

		return MakeRequest(webserverAddr, std::to_string(webserverPort),
							http::verb::get, "/api/list-sessions", std::move(cookies), "", cookieStorage);
	}

	std::future<Response> NetcodeNetworkModule::Status() {
		std::string cookies = cookieStorage.GetCookies();

		return MakeRequest(webserverAddr, std::to_string(webserverPort),
			http::verb::get, "/api/status", std::move(cookies), "", cookieStorage);
	}

	std::future<Response> NetcodeNetworkModule::Login(const std::wstring & username, const std::wstring & password) {
		std::string cookies = cookieStorage.GetCookies();
		
		json11::Json::object obj;
		obj["username"] = Egg::Utility::ToNarrowString(username);
		obj["password"] = Egg::Utility::ToNarrowString(password);

		json11::Json js = obj;
		std::string body = js.dump();

		return MakeRequest(webserverAddr, std::to_string(webserverPort),
			http::verb::post, "/api/login", std::move(cookies), std::move(body), cookieStorage);
	}
}
