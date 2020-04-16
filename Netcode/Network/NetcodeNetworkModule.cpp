#include "NetcodeNetworkModule.h"
#include "ClientSession.h"
#include "ServerSession.h"
#include "../Utility.h"
#include <json11/json11.hpp>

namespace Netcode::Module {

	void NetcodeNetworkModule::Start(AApp * app)
	{
	}

	void NetcodeNetworkModule::Shutdown()
	{
	}

	Network::GameSessionRef NetcodeNetworkModule::CreateServer(Network::Config config)
	{
		context.Start(config.server.workerThreadCount);
		return std::make_shared<Network::ServerSession>(context.GetImpl(), config);
	}

	Network::GameSessionRef NetcodeNetworkModule::CreateClient(Network::Config config)
	{
		context.Start(config.server.workerThreadCount);
		return std::make_shared<Network::ClientSession>(context.GetImpl(), config);
	}

	void NetcodeNetworkModule::Configure(Network::Config config)
	{
		this->config = std::move(config);
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

		return httpSession->MakeRequest(config.web.hostAddress, std::to_string(config.web.hostPort), "/api/login", Network::http::verb::post, cookiesCache, std::move(body));
	}

	std::future<Response> NetcodeNetworkModule::QueryServers()
	{
		if(httpSession == nullptr) {
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		return httpSession->MakeRequest(config.web.hostAddress, std::to_string(config.web.hostPort), "/api/list-sessions", Network::http::verb::post, cookiesCache, "");
	}

	std::future<Response> NetcodeNetworkModule::Status()
	{
		if(httpSession == nullptr) {
			httpSession = std::make_shared<Network::HttpSession>(context.GetImpl());
		}

		return httpSession->MakeRequest(config.web.hostAddress, std::to_string(config.web.hostPort), "/api/status", Network::http::verb::post, cookiesCache, "");
	}

}
