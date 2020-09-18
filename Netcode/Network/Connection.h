#pragma once

#include "NetworkCommon.h"
#include <Netcode/Sync/SlimReadWriteLock.h>
#include <Netcode/System/SystemClock.h>

namespace Netcode::Network {

	enum class ConnectionState : uint32_t {
		INACTIVE,
		TIMEOUT,
		CONNECTING,
		SYNCHRONIZING,
		ESTABLISHED
	};

	class EncryptionContext {
		// subject to future work
	};

	class Connection {
	public:
		NETCODE_CONSTRUCTORS_NO_COPY_NO_MOVE(Connection);
		
		// time of the authorization time
		Timestamp authorizedAt;

		// time of the last received message
		Timestamp lastReceivedAt;

		// endpoint seen by the host
		UdpEndpoint publicEndpoint;

		// endpoint proposed by the peer
		UdpEndpoint privateEndpoint;

		// endpoint where the connection would be handled
		UdpEndpoint liveEndpoint;

		// cached playerdata for the host
		PlayerDbDataRow playerData;

		// last arrived packet id
		int32_t packetSequenceId;

		int32_t peerPacketSequenceId;

		// state of the connection
		ConnectionState state;

		// nonce value attached to this player
		std::string nonce;
		
		// optional connection socket
		std::unique_ptr<UdpSocket> socket;

		// optional encryption data attached to this user
		std::unique_ptr<EncryptionContext> encryptionContext;
	};

}
