#pragma once 

#include "NetworkDecl.h"
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

		virtual Network::CompletionToken<Network::Response> Login(const std::wstring & username, const std::wstring & password) override;
		virtual Network::CompletionToken<Network::Response> QueryServers() override;
		virtual Network::CompletionToken<Network::Response> Status() override;
	};

}
