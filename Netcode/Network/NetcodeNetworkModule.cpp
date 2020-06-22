#include "NetcodeNetworkModule.h"
#include "ClientSession.h"
#include "ServerSession.h"
#include "../Utility.h"
#include <json11.hpp>

namespace Netcode::Module {

	void NetcodeNetworkModule::Start(AApp * app, Netcode::Config * config)
	{
		this->config = config;
	}

	void NetcodeNetworkModule::Shutdown()
	{
		this->config = nullptr;
	}

	Network::GameSessionRef NetcodeNetworkModule::CreateServer()
	{
		context.Start(config->network.server.workerThreadCount);
		return std::make_shared<Network::ServerSession>(context.GetImpl(), config);
	}

	Network::GameSessionRef NetcodeNetworkModule::CreateClient()
	{
		context.Start(config->network.server.workerThreadCount);
		return std::make_shared<Network::ClientSession>(context.GetImpl(), config);
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

		return httpSession->MakeRequest(config->network.web.hostname, std::to_string(config->network.web.port), "/api/login", Network::http::verb::post, cookiesCache, std::move(body));
	}

	std::future<Response> NetcodeNetworkModule::QueryServers()
	{
		if(httpSession == nullptr) {
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		return httpSession->MakeRequest(config->network.web.hostname, std::to_string(config->network.web.port), "/api/list-sessions", Network::http::verb::post, cookiesCache, "");
	}

	std::future<Response> NetcodeNetworkModule::Status()
	{
		if(httpSession == nullptr) {
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		return httpSession->MakeRequest(config->network.web.hostname, std::to_string(config->network.web.port), "/api/status", Network::http::verb::post, cookiesCache, "");
	}

}
