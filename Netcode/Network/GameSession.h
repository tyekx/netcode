#pragma once 

#include <NetcodeProtocol/netcode.pb.h>
#include <NetcodeFoundation/Macros.h>

#include "Connection.h"
#include "NetworkCommon.h"


namespace Netcode::Network {

	class SessionBase : public std::enable_shared_from_this<SessionBase> {
	public:
		virtual NETCODE_CONSTRUCTORS_NO_COPY(SessionBase);
		
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
