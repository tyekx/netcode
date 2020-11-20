#include "MatchmakerSession.h"
#include "Service.h"
#include "Connection.h"
#include "MysqlSession.h"

namespace Netcode::Network {
	
	void MatchmakerSession::Stop() {

	}

	void MatchmakerSession::Start() {
		UdpSocket socket{ ioContext };

		ssl_ptr<SSL_CTX> serverCtx{ SSL_CTX_new(DTLSv1_2_server_method()) };

		if(SSL_CTX_use_certificate_chain_file(serverCtx.get(), "server.crt") != 1) {
			Log::Error("Failed to load matchmaker certificate");
		}
		
		if(SSL_CTX_use_PrivateKey_file(serverCtx.get(), "server.key", SSL_FILETYPE_PEM) != 1) {
			Log::Error("Failed to load matchmaker private key");
		}

		if(SSL_CTX_check_private_key(serverCtx.get()) != 1) {
			Log::Error("Matchmaker private key check failed");
		}

		SslInitializeCookies();

		SSL_CTX_set_verify(serverCtx.get(), SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, SslVerifyCertificate);
		SSL_CTX_set_cookie_generate_cb(serverCtx.get(), SslGenerateCookie);
		SSL_CTX_set_cookie_verify_cb(serverCtx.get(), SslVerifyCookie);

		constexpr uint32_t SSL_OPTIONS = SSL_OP_NO_SSLv2 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_3 | SSL_OP_NO_COMPRESSION;

		SSL_CTX_set_options(serverCtx.get(), SSL_OPTIONS);

		if(SSL_CTX_set_cipher_list(serverCtx.get(), DTLS_CIPHERS) != 1) {
			Log::Error("Server: failed to set cipherlist");
		}

		uint32_t portHint = Config::Get<uint16_t>(L"network.matchmaker.port:u16");
		std::string localAddr = Utility::ToNarrowString(Config::Get<std::wstring>(L"network.matchmaker.localAddr:string"));
		
		boost::system::error_code ec;
		IpAddress addr = boost::asio::ip::make_address(localAddr, ec);

		if(ec) {
			Log::Error("Failed to parse supplied IP address");
			return;
		}
		
		std::vector<Interface> ifaces = GetCompatibleInterfaces(addr);

		if(ifaces.empty()) {
			Log::Error("No suitable network interfaces found");
			return;
		}

		Interface iface = ifaces.front();

		ErrorCode bindResult = Bind(iface.address, socket, portHint);
		
		if(!bindResult) {
			Log::Error("Failed to bind to network interface");
			return;
		}

		if(portHint > std::numeric_limits<uint16_t>::max()) {
			Log::Error("[Network] [Server] Failed to bind game port socket");
			return;
		}

		Config::Set<uint16_t>(L"network.matchmaker.port:u16", static_cast<uint16_t>(portHint));

		service = std::make_shared<NetcodeService>(ioContext, std::move(socket), static_cast<uint16_t>(iface.mtu), nullptr, std::move(serverCtx));
		service->Host();
	}

}
