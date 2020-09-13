#pragma once 

#include <NetcodeProtocol/netcode.pb.h>
#include <NetcodeFoundation/Macros.h>

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

		virtual void SwapBuffers(std::vector<Protocol::ServerUpdate> & game) = 0;
		virtual void Update(Protocol::ClientUpdate message) = 0;
		virtual void Connect(std::string address, uint16_t port, std::string hash) = 0;
		virtual void Disconnect() = 0;
	};
	
	class ServerSessionBase : public SessionBase {
	public:
		virtual NETCODE_CONSTRUCTORS_NO_COPY(ServerSessionBase);

		virtual void SwapBuffers(std::vector<Protocol::ClientUpdate> & game) = 0;
		virtual void Update(int32_t subjectId, Protocol::ServerUpdate serverUpdate) = 0;
	};

}
