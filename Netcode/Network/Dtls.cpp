#include "Dtls.h"
#include "MtuValue.hpp"
#include "Service.h"
#include "NetworkErrorCode.h"


namespace Netcode::Network {

	DtlsRecordLayer DtlsRecordLayer::Load(const DtlsRecordLayerWire * source) {
		DtlsRecordLayer recLay;
		recLay.contentType = source->contentType;
		recLay._struct_padding0 = 0;
		recLay.protocolVersion = ntohs(source->protocolVersion);
		recLay.epoch = ntohs(source->epoch);
		recLay.sequenceNumber = ntohll(source->sequenceAndLength & (~0xFFFF)) >> 16;
		recLay.length = static_cast<uint16_t>(ntohll(source->sequenceAndLength));
		return recLay;
	}

	DtlsRoute * DtlsService::ServerListen(NetcodeService * service, UdpPacket * packet) {
		if(serverContext == nullptr) {
			return nullptr;
		}

		if(listener == nullptr) {
			listener.reset(SSL_new(serverContext.get()));
		}

		SSL * ssl = listener.get();

		if(SSL_set_ex_data(listener.get(), 0, packet) != 1) {
			Log::Error("SSL_set_ex_data() failed");
			return nullptr;
		}

		BIO * outBio = nullptr;
		std::error_code ec = SslListenProceed(ssl, packet, &outBio);

		if(ec == NetworkErrc::SSL_CONTINUATION_NEEDED) {
			// SSL listen is stateless, so we dont care about the results
			service->Send(nullptr, nullptr, packet->endpoint, ssl_ptr<BIO>{ outBio }, MtuValue::DEFAULT);
		}

		// successfully accepted a connection
		if(!ec) {
			DtlsRoute * route = router.Add();

			if(route == nullptr) {
				SSL_clear(ssl);
				SSL_set_mtu(ssl, MtuValue::DEFAULT);
			} else {
				Timestamp localNow = SystemClock::LocalNow();
				route->mtu = MtuValue::DEFAULT;
				route->lastResentAt = localNow;
				route->lastReceivedAt = localNow;
				route->state = DtlsRouteState::SERVER_ACCEPT;
				route->endpoint = packet->endpoint;
				route->ssl = std::move(listener);
			}

			return route;
		}

		return nullptr;
	}

	void DtlsService::ServerAccept(NetcodeService * service, DtlsRoute * route, NetAllocator * alloc, UdpPacket * packet) {
		BIO * outBio = nullptr;
		std::error_code ec = SslAcceptProceed(route->ssl.get(), packet, &outBio);

		if(outBio != nullptr) {
			service->Send(alloc->shared_from_this(), nullptr, route, ssl_ptr<BIO>{ outBio });
		}

		if(!ec) {
			route->state = DtlsRouteState::ESTABLISHED;
		}
	}

	struct DtlsResendContext : std::enable_shared_from_this<DtlsResendContext> {
		Ref<NetAllocator> alloc;
		ssl_ptr<BIO> content;
		WaitableTimer * timer;
		DtlsRoute * route;
		NetcodeService * service;
		uint32_t numAttempts;
		uint32_t maxAttempts;

		void Attempt() {
			if(numAttempts >= maxAttempts) {
				return;
			}

			numAttempts++;

			ssl_ptr<BIO> c{ content.get() };
			BIO_up_ref(c.get());
			service->Send(alloc, nullptr, route->endpoint, std::move(c), MtuValue::DEFAULT);
		}
	};

	void DtlsService::ClientConnect(NetcodeService * service, DtlsRoute * route, NetAllocator * alloc, UdpPacket * packet) {
		BIO * outBio = nullptr;
		std::error_code ec = SslClientProceed(route->ssl.get(), packet, &outBio);
		
		if(ec == NetworkErrc::SSL_CONTINUATION_NEEDED) {
			CompletionToken<TrResult> ct = alloc->MakeCompletionToken<TrResult>();
			ct->Then([this](const TrResult & tr) -> void {
				if(tr.errorCode) {
					pendingConnection->Set(DtlsConnectResult{ tr.errorCode, nullptr });
					pendingConnection.reset();
				}
			});
			
			service->Send(alloc->shared_from_this(), std::move(ct), route->endpoint, ssl_ptr<BIO>{ outBio }, MtuValue::DEFAULT);
		}

		if(!ec) {
			route->state = DtlsRouteState::ESTABLISHED;

			DtlsConnectResult connRes;
			connRes.errorCode = ec;
			connRes.route = route;
			pendingConnection->Set(connRes);
			pendingConnection.reset();
		}
	}

	DtlsRoute * DtlsService::HandlePacket(NetcodeService * service, const DtlsRecordLayer & firstRecord, NetAllocator * alloc, UdpPacket * packet) {
		DtlsRoute * route = router.Find(packet->endpoint);

		if(route == nullptr) {
			route = ServerListen(service, packet);
		}

		if(route != nullptr) {
			if(route->state == DtlsRouteState::ESTABLISHED) {
				if(firstRecord.contentType == DtlsContentType::APPLICATION) {
					if(firstRecord.length + MtuValue::DTLS_RL_HEADER_SIZE == packet->GetSize()) {
						return route;
					}
				}

				//TODO: handle shutdown here

				return nullptr;
			}

			if(route->state == DtlsRouteState::SERVER_ACCEPT) {
				ServerAccept(service, route, alloc, packet);
			}

			if(route->state == DtlsRouteState::CLIENT_CONNECT) {
				ClientConnect(service, route, alloc, packet);
			}
		}

		return nullptr;
	}

	CompletionToken<DtlsConnectResult> DtlsService::InitConnect(NetcodeService * service, NetAllocator * alloc, const UdpEndpoint & target) {
		if(clientContext == nullptr) {
			Log::Error("ClientContext was not initialized");
			return nullptr;
		}

		DtlsRoute * route = router.Add();

		if(route == nullptr) {
			Log::Error("Routing table is full");
			return nullptr;
		}

		if(pendingConnection != nullptr) {
			Log::Error("Already has a pending connection");
			return nullptr;
		}
		
		pendingConnection = alloc->MakeCompletionToken<DtlsConnectResult>();

		WaitableTimer * timer = alloc->Make<WaitableTimer>(service->GetIOContext());
		timer->expires_after(std::chrono::seconds(3));
		timer->async_wait([pc = pendingConnection](ErrorCode ec) -> void {
			if(!ec) {
				DtlsConnectResult cr;
				cr.route = nullptr;
				cr.errorCode = make_error_code(NetworkErrc::CLIENT_TIMEOUT);
				if(pc != nullptr) {
					pc->Set(cr);
				}
			}
		});

		Timestamp localNow = SystemClock::LocalNow();
		route->lastResentAt = localNow;
		route->lastReceivedAt = localNow;
		route->endpoint = target;
		route->mtu = MtuValue::DEFAULT;
		route->state = DtlsRouteState::CLIENT_CONNECT;
		ssl_ptr<SSL> ssl{ SSL_new(clientContext.get()) };
		SSL_set_mtu(ssl.get(), MtuValue::DEFAULT);
		SSL_set_bio(ssl.get(), nullptr, BIO_new(BIO_s_mem()));
		SSL_set_connect_state(ssl.get());
		route->ssl = std::move(ssl);

		ClientConnect(service, route, alloc, nullptr);

		return pendingConnection;
	}
	
}
