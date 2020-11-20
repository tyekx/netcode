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

	void DtlsService::AsyncCheckTimeouts() {
		post(strand, [this]() {
			DtlsRoute * route = router.GetHead();
			Timestamp t0 = SystemClock::LocalNow();

			while(route != nullptr) {
				DtlsRoute * next = route->next;

				const DtlsRouteState state = route->state;

				if(state == DtlsRouteState::SERVER_ACCEPT) {
					const Timestamp t1 = route->lastReceivedAt;

					if(t1 < t0 && (t0 - t1) > std::chrono::seconds(1)) {
						router.Erase(route);
					}
				}

				if(state == DtlsRouteState::DISCONNECTED) {
					router.Erase(route);
				}
				
				route = next;
			}
		});
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
		SSL * ssl = route->ssl.get();

		const OSSL_HANDSHAKE_STATE beforeState = SSL_get_state(ssl);
		
		const std::error_code ec = SslAcceptProceed(ssl, packet, &outBio);

		const OSSL_HANDSHAKE_STATE afterState = SSL_get_state(ssl);

		if(beforeState == TLS_ST_SR_CLNT_HELLO && afterState == TLS_ST_SW_SRVR_DONE) {
			//TODO make sure this is safe, only safe if client cant transition between CLNT_HELLO and SRVR_DONE freely.
			//route->lastReceivedAt = SystemClock::LocalNow();
		}

		if(outBio != nullptr) {
			service->Send(alloc->shared_from_this(), nullptr, route, ssl_ptr<BIO>{ outBio });
		}

		if(!ec && afterState == TLS_ST_OK) {
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
		SSL * ssl = route->ssl.get();

		const OSSL_HANDSHAKE_STATE beforeState = SSL_get_state(ssl);
		const std::error_code ec = SslClientProceed(ssl, packet, &outBio);
		const OSSL_HANDSHAKE_STATE afterState = SSL_get_state(ssl);
		
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

		if(!ec && afterState == TLS_ST_OK) {
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

	void DtlsService::AsyncHandlePacket(NetcodeService * service, NetAllocator * alloc, UdpPacket * packet) {
		post(strand, [this, service, packet, al = alloc->shared_from_this()]() {
			if(packet->GetSize() < DTLS1_RT_HEADER_LENGTH) {
				return;
			}
			
			DtlsRecordLayer record = DtlsRecordLayer::Load(reinterpret_cast<const DtlsRecordLayerWire *>(packet->GetData()));
			
			DtlsRoute * route = HandlePacket(service, record, al.get(), packet);

			if(route != nullptr) {
				service->HandleRoutedMessage(al.get(), route, packet);
			}
		});
	}

	CompletionToken<DtlsConnectResult> DtlsService::InitConnect(NetcodeService * service, NetAllocator * alloc, const UdpEndpoint & target) {
		if(clientContext == nullptr) {
			Log::Error("ClientContext was not initialized");
			return nullptr;
		}

		if(pendingConnection != nullptr) {
			Log::Error("Already has a pending connection");
			return nullptr;
		}

		pendingConnection = alloc->MakeCompletionToken<DtlsConnectResult>();

		post(strand, [this, service, ep = target, al = alloc->shared_from_this()]() -> void {
			DtlsRoute * route = router.Add();

			if(route == nullptr) {
				Log::Error("Routing table is full");
				pendingConnection->Set(DtlsConnectResult{ make_error_code(NetworkErrc::SSL_ERROR), nullptr });
				return;
			}

			WaitableTimer * timer = al->Make<WaitableTimer>(service->GetIOContext());
			
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
			route->endpoint = ep;
			route->mtu = MtuValue::DEFAULT;
			route->state = DtlsRouteState::CLIENT_CONNECT;
			ssl_ptr<SSL> ssl{ SSL_new(clientContext.get()) };
			SSL_set_mtu(ssl.get(), MtuValue::DEFAULT);
			SSL_set_bio(ssl.get(), nullptr, BIO_new(BIO_s_mem()));
			SSL_set_connect_state(ssl.get());
			route->ssl = std::move(ssl);

			ClientConnect(service, route, al.get(), nullptr);
		});
		

		return pendingConnection;
	}
	
}
