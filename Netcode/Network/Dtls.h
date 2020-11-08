#pragma once

#include <Netcode/HandleDecl.h>
#include <Netcode/System/SystemClock.h>
#include "NetworkDecl.h"
#include "SslUtil.h"
#include "NetworkErrorCode.h"
#include "CompletionToken.h"

namespace Netcode::Network {

	enum class DtlsContentType : uint8_t {
		CHANGE_CIPHER_SPEC = 20,
		ALERT = 21,
		HANDSHAKE = 22,
		APPLICATION = 23
	};

	//!!! on-wire format, everything will be in network byte order!!!
#pragma pack(push, 1)
	struct DtlsRecordLayerWire {
		DtlsContentType contentType;
		uint16_t protocolVersion;
		uint16_t epoch;
		uint64_t sequenceAndLength;
	};
#pragma pack(pop)

	// parsed format
	struct DtlsRecordLayer {
		DtlsContentType contentType;
		uint8_t _struct_padding0;
		uint16_t protocolVersion;
		uint16_t epoch;
		uint16_t length;
		uint64_t sequenceNumber;

		static DtlsRecordLayer Load(const DtlsRecordLayerWire * source);
	};

	enum class DtlsRouteState : uint16_t {
		UNDEFINED, CLIENT_CONNECT, SERVER_ACCEPT, ESTABLISHED
	};

	struct DtlsRoute {
		ssl_ptr<SSL> ssl;
		Timestamp lastReceivedAt;
		Timestamp lastResentAt;
		UdpEndpoint endpoint;
		DtlsRouteState state;
		uint16_t mtu;
		DtlsRoute * next;

		DtlsRoute() : ssl{}, lastReceivedAt{}, lastResentAt{}, endpoint{}, state{ DtlsRouteState::UNDEFINED }, mtu{ 0 }, next{ nullptr } {}
	};

	class NetAllocator;
	class NetcodeService;
	
	class DtlsRouter {
		DtlsRoute pool[64];
		DtlsRoute * storage;
		DtlsRoute * head;
	public:
		DtlsRouter() : pool{}, storage{}, head{} {
			// leave the last item's next to nullptr
			for(uint32_t i = 0; i < 63; i++) {
				pool[i].next = pool + i + 1;
			}
			storage = pool;
		}

		// gets the head pointer to iterate over the active routes
		DtlsRoute * GetHead() const {
			return head;
		}

		DtlsRoute * Add() {
			if(storage == nullptr) {
				return nullptr;
			}

			// pop head
			DtlsRoute * re = storage;
			storage = storage->next;

			// add to head
			re->next = head;
			head = re;

			// signal ok
			return re;
		}

		DtlsRoute * Find(const UdpEndpoint & endpoint) {
			for(DtlsRoute * it = head; it != nullptr; it = it->next) {
				if(it->endpoint == endpoint) {
					return it;
				}
			}
			return nullptr;
		}

		void Erase(DtlsRoute * route) {
			// are we even the owners? handles nullptr case aswell
			if(route < pool || route >= (pool + 64)) {
				return;
			}

			route->~DtlsRoute();
			new (route) DtlsRoute{};

			if(route == head) {
				head = head->next;
			} else {
				DtlsRoute * prev = head;
				for(DtlsRoute * it = head->next; it != nullptr; it = it->next) {
					if(it == route) {
						prev->next = it->next;
						break;
					}
					prev = it;
				}
			}

			// push it back to the storage
			route->next = storage;
			storage = route;
		}
	};

	struct DtlsConnectResult {
		ErrorCode errorCode;
		DtlsRoute * route;
	};

	class DtlsService {
		DtlsRouter router;
		ssl_ptr<SSL_CTX> clientContext;
		ssl_ptr<SSL_CTX> serverContext;
		ssl_ptr<SSL> listener;

		CompletionToken<DtlsConnectResult> pendingConnection;
	public:

		SSL_CTX* GetServerContext() {
			return serverContext.get();
		}

		SSL_CTX* GetClientContext() {
			return clientContext.get();
		}

		DtlsService(ssl_ptr<SSL_CTX> clientContext, ssl_ptr<SSL_CTX> serverContext) :
			router{}, clientContext{ std::move(clientContext) }, serverContext{ std::move(serverContext) } {

		}

		DtlsRoute * ServerListen(NetcodeService * service, UdpPacket * packet);

		void ServerAccept(NetcodeService * service, DtlsRoute * route, NetAllocator * alloc, UdpPacket * packet);

		void ClientConnect(NetcodeService * service, DtlsRoute * route, NetAllocator * alloc, UdpPacket * packet);

		DtlsRoute * HandlePacket(NetcodeService * service, const DtlsRecordLayer & firstRecord, NetAllocator * alloc, UdpPacket * packet);

		CompletionToken<DtlsConnectResult> InitConnect(NetcodeService * service, NetAllocator * alloc, const UdpEndpoint & target);

	};

}
