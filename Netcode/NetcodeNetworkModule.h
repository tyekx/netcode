#pragma once

#include <NetcodeLib/Cookie.h>
#include <boost/beast.hpp>
#include <mutex>

#include "Modules.h"


namespace Egg::Module {

	namespace http = boost::beast::http;

	class CookieStorage {
		std::map<std::string, Cookie> cookies;
		std::string cookiesCache;
		mutable std::mutex mutex;
	public:
		std::string GetCookies() const;
		void SetCookies(std::vector<Cookie> cks);
		Cookie GetCookie(const std::string & key) const;
	};

	class NetcodeNetworkModule : public INetworkModule {
		std::string webserverAddr;
		int webserverPort;
		CookieStorage cookieStorage;

	public:
		virtual void Start(AApp * app) override;

		virtual void Shutdown() override;

		virtual void SetWebserverAddress(std::string ipAddr, int port) override;

		virtual Cookie GetCookie(const std::string & key) override;

		virtual std::future<Response> QueryServers() override;

		virtual std::future<Response> Status() override;

		virtual std::future<Response> Login(const std::wstring & username, const std::wstring & password) override;
	};

}
