#pragma once 

#include <Netcode/Modules.h>
#include "NetworkCommon.h"
#include "HttpSession.h"
#include "Cookie.h"

namespace Netcode::Module {

	class NetcodeNetworkModule : public INetworkModule {
		Network::NetworkContext context;
		Ref<Network::HttpSession> httpSession;
		std::map<std::string, Network::Cookie> cookieStorage;
		std::string cookiesCache;

	public:
		virtual void Start(AApp * app) override;
		virtual void Shutdown() override;

		virtual Ref<Network::ServerSessionBase> CreateServer() override;
		virtual Ref<Network::ClientSessionBase> CreateClient() override;

		virtual Network::Cookie GetCookie(const std::string & key) override;
		virtual void SetCookie(const Network::Cookie & cookie) override;

		virtual std::future<Network::Response> Login(const std::wstring & username, const std::wstring & password) override;
		virtual std::future<Network::Response> QueryServers() override;
		virtual std::future<Network::Response> Status() override;
	};

}
