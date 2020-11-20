#include <boost/asio.hpp>

#include "ServerSession.h"
#include "Macros.h"
#include <Netcode/Utility.h>
#include <Netcode/Logger.h>
#include <Netcode/Config.h>

#include <openssl/rand.h>

#include "Service.h"

namespace Netcode::Network {

	ServerSession::ServerSession(boost::asio::io_context & ioc) : ioContext{ ioc } {
	}

	void ServerSession::Start() {
		UdpSocket gameSocket{ ioContext };

		ssl_ptr<SSL_CTX> serverCtx{ SSL_CTX_new(DTLSv1_2_server_method()) };
		//TODO: add DTLS client functionality to server
		//ssl_ptr<SSL_CTX> clientCtx{ SSL_CTX_new(DTLSv1_2_client_method()) };
		//TODO load cert and PK file path from configuration
		//TODO load file into memory
		//TODO from memory: https://stackoverflow.com/questions/3810058/read-certificate-files-from-memory-instead-of-a-file-using-openssl
		if(SSL_CTX_use_certificate_chain_file(serverCtx.get(), "server.crt") != 1) {
			Log::Error("Failed to load server certificate");
		}

		//TODO from memory: https://stackoverflow.com/questions/11886262/reading-public-private-key-from-memory-with-openssl/11888897
		if(SSL_CTX_use_PrivateKey_file(serverCtx.get(), "server.key", SSL_FILETYPE_PEM) != 1) {
			Log::Error("Failed to load server private key");
		}

		if(SSL_CTX_check_private_key(serverCtx.get()) != 1) {
			Log::Error("Server private key check failed");
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

		const uint32_t gracePeriodMs = Config::Get<uint32_t>(L"network.protocol.gracePeriodMs:u32");
		uint32_t gamePort = Config::Get<uint16_t>(L"network.server.port:u16");
		std::string selfAddr = Utility::ToNarrowString(Config::Get<std::wstring>(L"network.server.selfAddress:string"));

		boost::system::error_code ec;
		IpAddress addr = boost::asio::ip::address::from_string(selfAddr, ec);

		RETURN_ON_ERROR(ec, "Failed to parse supplied IP address: {0}")

		std::vector<Interface> ifaces = GetCompatibleInterfaces(addr);

		RETURN_IF_AND_LOG_ERROR(ifaces.empty(), "No suitable network interface found");

		Interface iface = ifaces.front();

		RETURN_ON_ERROR(ec, "[Network] [Server] invalid configuration value: {0}")

		Bind(iface.address, gameSocket, gamePort);

		if(gamePort > std::numeric_limits<uint16_t>::max()) {
			Log::Error("[Network] [Server] Failed to bind game port socket");
			return;
		}
		
		Config::Set<uint16_t>(L"network.server.port:u16", static_cast<uint16_t>(gamePort));

		Log::Info("[Network] [Server] Started on port: {0}", Config::Get<uint16_t>(L"network.server.port:u16"));

		service = std::make_shared<NetcodeService>(ioContext, std::move(gameSocket), static_cast<uint16_t>(iface.mtu), nullptr, std::move(serverCtx));
		service->Host();
	}

	void ServerSession::Stop()
	{
		service->Close();
		service.reset();
	}

}
