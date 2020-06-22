#pragma once 

#include "../Modules.h"
#include "NetworkCommon.h"
#include "HttpSession.h"

namespace Netcode::Module {

	class NetcodeNetworkModule : public INetworkModule {
		Network::NetworkContext context;
		Network::HttpSessionRef httpSession;
		std::map<std::string, Network::Cookie> cookieStorage;
		std::string cookiesCache;

	public:
		virtual void Start(AApp * app) override;
		virtual void Shutdown() override;

		virtual Network::GameSessionRef CreateServer() override;
		virtual Network::GameSessionRef CreateClient() override;

		virtual Network::Cookie GetCookie(const std::string & key) override;
		virtual void SetCookie(const Network::Cookie & cookie) override;

		virtual std::future<Response> Login(const std::wstring & username, const std::wstring & password) override;
		virtual std::future<Response> QueryServers() override;
		virtual std::future<Response> Status() override;
	};

}
