#include "NetcodeNetworkModule.h"
#include "ClientSession.h"
#include "ServerSession.h"
#include "Cookie.h"

#include <json11.hpp>

#include <Netcode/Utility.h>
#include <Netcode/Config.h>

#include "Response.hpp"

namespace Netcode::Module {

	void NetcodeNetworkModule::Start(AApp * app) {
	}

	void NetcodeNetworkModule::Shutdown() {
	}

	Ref<Network::ServerSessionBase> NetcodeNetworkModule::CreateServer()
	{
		context.Start(Config::Get<uint32_t>(L"network.server.workerThreadCount:u32"));
		return std::make_shared<Network::ServerSession>(context.GetImpl());
	}

	Ref<Network::ClientSessionBase> NetcodeNetworkModule::CreateClient()
	{
		context.Start(Config::Get<uint32_t>(L"network.client.workerThreadCount:u32"));
		return std::make_shared<Network::ClientSession>(context.GetImpl());
	}

	void NetcodeNetworkModule::EraseCookie(const std::string & key)
	{
		if(auto it = cookieStorage.find(key); it != cookieStorage.end()) {
			cookieStorage.erase(it);
		}

		cookiesCache.clear();

		for(const auto & i : cookieStorage) {
			std::string s = i.second.GetCookieString();
			if(!cookiesCache.empty()) {
				cookiesCache += "; ";
			}
			cookiesCache += s;
		}
	}

	Network::Cookie NetcodeNetworkModule::GetCookie(const std::string & key)
	{
		if(auto it = cookieStorage.find(key); it != cookieStorage.end()) {
			return it->second;
		}

		return Network::Cookie();
	}

	void NetcodeNetworkModule::SetCookie(const Network::Cookie & cookie)
	{
		cookieStorage[cookie.GetName()] = cookie;

		cookiesCache.clear();

		for(const auto & i : cookieStorage) {
			std::string s = i.second.GetCookieString();
			if(!cookiesCache.empty()) {
				cookiesCache += "; ";
			}
			cookiesCache += s;
		}
	}

	Network::CompletionToken<Network::Response> NetcodeNetworkModule::Login(const std::wstring & username, const std::wstring & password)
	{
		if(httpSession == nullptr) {
			context.Start(Config::Get<uint32_t>(L"network.client.workerThreadCount:u32"));
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		json11::Json json = json11::Json::object{
			{ "username", Utility::ToNarrowString(username) },
			{ "password", Utility::ToNarrowString(password) }
		};

		std::string body = json.dump();

		return httpSession->MakeRequest(Utility::ToNarrowString(Config::Get<std::wstring>(L"network.web.hostname:string")),
			std::to_string(Config::Get<uint16_t>(L"network.web.port:u16")), "/api/login", Network::http::verb::post, cookiesCache, std::move(body));
	}

	Network::CompletionToken<Network::Response> NetcodeNetworkModule::QueryServers()
	{
		if(httpSession == nullptr) {
			context.Start(Config::Get<uint32_t>(L"network.client.workerThreadCount:u32"));
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		return httpSession->MakeRequest(Utility::ToNarrowString(Config::Get<std::wstring>(L"network.web.hostname:string")),
			std::to_string(Config::Get<uint16_t>(L"network.web.port:u16")), "/api/list-sessions", Network::http::verb::get, cookiesCache, "");
	}

	Network::CompletionToken<Network::Response> NetcodeNetworkModule::Status()
	{
		if(httpSession == nullptr) {
			context.Start(Config::Get<uint32_t>(L"network.client.workerThreadCount:u32"));
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		return httpSession->MakeRequest(Utility::ToNarrowString(Config::Get<std::wstring>(L"network.web.hostname:string")),
			std::to_string(Config::Get<uint16_t>(L"network.web.port:u16")), "/api/status", Network::http::verb::get, cookiesCache, "");
	}

	Network::CompletionToken<Network::Response> NetcodeNetworkModule::Logout() {
		if(httpSession == nullptr) {
			context.Start(Config::Get<uint32_t>(L"network.client.workerThreadCount:u32"));
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		return httpSession->MakeRequest(Utility::ToNarrowString(Config::Get<std::wstring>(L"network.web.hostname:string")),
			std::to_string(Config::Get<uint16_t>(L"network.web.port:u16")), "/api/logout", Network::http::verb::get, cookiesCache, "");
	}

}
