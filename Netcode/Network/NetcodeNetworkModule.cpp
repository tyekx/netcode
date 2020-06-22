#include "NetcodeNetworkModule.h"
#include "ClientSession.h"
#include "ServerSession.h"
#include "../Utility.h"
#include <json11.hpp>

#include "../Config.h"

namespace Netcode::Module {

	void NetcodeNetworkModule::Start(AApp * app) {
	}

	void NetcodeNetworkModule::Shutdown() {
	}

	Network::GameSessionRef NetcodeNetworkModule::CreateServer()
	{
		context.Start(Config::Get<uint32_t>("network.server.workerThreadCount:u32"));
		return std::make_shared<Network::ServerSession>(context.GetImpl());
	}

	Network::GameSessionRef NetcodeNetworkModule::CreateClient()
	{
		context.Start(Config::Get<uint32_t>("network.client.workerThreadCount:u32"));
		return std::make_shared<Network::ClientSession>(context.GetImpl());
	}

	Network::Cookie NetcodeNetworkModule::GetCookie(const std::string & key)
	{
		auto it = cookieStorage.find(key);

		if(it != cookieStorage.end()) {
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

	std::future<Response> NetcodeNetworkModule::Login(const std::wstring & username, const std::wstring & password)
	{
		if(httpSession == nullptr) {
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		json11::Json json = json11::Json::object{
			{ "username", Utility::ToNarrowString(username) },
			{ "password", Utility::ToNarrowString(password) }
		};

		std::string body = json.dump();

		return httpSession->MakeRequest(Config::Get<std::string>("network.web.hostname:string"), std::to_string(Config::Get<uint16_t>("network.web.port:u16")), "/api/login", Network::http::verb::post, cookiesCache, std::move(body));
	}

	std::future<Response> NetcodeNetworkModule::QueryServers()
	{
		if(httpSession == nullptr) {
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		return httpSession->MakeRequest(Config::Get<std::string>("network.web.hostname:string"), std::to_string(Config::Get<uint16_t>("network.web.port:u16")), "/api/list-sessions", Network::http::verb::post, cookiesCache, "");
	}

	std::future<Response> NetcodeNetworkModule::Status()
	{
		if(httpSession == nullptr) {
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		return httpSession->MakeRequest(Config::Get<std::string>("network.web.hostname:string"), std::to_string(Config::Get<uint16_t>("network.web.port:u16")), "/api/status", Network::http::verb::post, cookiesCache, "");
	}

}
