#pragma once 

#include <NetcodeProtocol/netcode.pb.h>
#include <NetcodeFoundation/Macros.h>

#include "Connection.h"
#include "NetworkCommon.h"


namespace Netcode::Network {

	//constexpr static const char * DTLS_CIPHERS = "ECDHE-RSA-NULL-SHA:@SECLEVEL=0";
	constexpr static const char * DTLS_CIPHERS = "ECDHE-RSA-AES128-GCM-SHA256";

	class SessionBase : public std::enable_shared_from_this<SessionBase> {
	public:
		virtual NETCODE_CONSTRUCTORS_NO_COPY(SessionBase);

		virtual boost::asio::io_context & GetIOContext() = 0;
		virtual void Start() = 0;
		virtual void Stop() = 0;
	};

	class ClientSessionBase : public SessionBase {
	public:
		virtual NETCODE_CONSTRUCTORS_NO_COPY(ClientSessionBase);

	};
	
	class ServerSessionBase : public SessionBase {
	public:
		virtual NETCODE_CONSTRUCTORS_NO_COPY(ServerSessionBase);

	};

}
