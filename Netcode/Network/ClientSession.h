#pragma once 

#include <Netcode/ModulesConfig.h>
#include "GameSession.h"
#include "NetworkCommon.h"
#include <boost/asio.hpp>

#include <NetcodeFoundation/Enum.hpp>
#include "NetcodeProtocol/header.pb.h"

namespace Netcode::Network {

	enum class NatType : unsigned {
		UNDETECTED = 0x0,
		DETECTED = 0x1,
		OPEN = 0x10,
		RESTRICTED = 0x20,
		CLOSED = 0x40,
		ASSUMED_RESTRICTED = ( RESTRICTED | UNDETECTED ),
		ASSUMED_CLOSED = ( CLOSED | UNDETECTED ),
		ASSUMED_OPEN = ( OPEN | UNDETECTED ),
		CONFIRMED_RESTRICTED = ( RESTRICTED | DETECTED ),
		CONFIRMED_CLOSED = ( CLOSED | DETECTED ),
		CONFIRMED_OPEN = ( OPEN | DETECTED )
	};

	NETCODE_ENUM_CLASS_OPERATORS(NatType)

	class NtpClockFilter {
		
		struct Tuple {
			double offset;
			double delay;
			Timestamp time;
		};

		constexpr static double PRECISION = 1.0 / (1 << 18);

		Tuple buffer[8];
		Timestamp lastValidPacket;
		
		void ClockFilter() {
			static Tuple* tmpBuffer[8] = {
				&buffer[0],
				&buffer[1],
				&buffer[2],
				&buffer[3],
				&buffer[4],
				&buffer[5],
				&buffer[6],
				&buffer[7],
			};

			std::sort(std::begin(tmpBuffer), std::end(tmpBuffer), [](const Tuple * lhs, const Tuple * rhs) -> bool {
				return lhs->delay < rhs->delay;
			});

			const double currentBestOffset = buffer[1].offset;
			double jitter = 0.0;
			
			const Tuple* f0 = tmpBuffer[0];
			
			for(int i = 0; i < 8;i++) {
				jitter += (tmpBuffer[i]->offset - f0->offset) * (tmpBuffer[i]->offset - f0->offset);
			}
			jitter = std::max(sqrt(jitter), PRECISION);

			// do not reuse packets
			if(f0->time <= lastValidPacket) {
				return;
			}

			// check if its even an upgrade
			if(fabs(f0->offset - currentBestOffset) > 3.0 * jitter) {
				return;
			}

			// accept it as an upgrade
			lastValidPacket = f0->time;
		}

		constexpr double DurationToDouble(const Duration & d) {
			return std::chrono::duration<double, std::milli>(d).count();
		}
		
	public:
		NtpClockFilter() : buffer{} {
			double maxDispersion = DurationToDouble(std::chrono::seconds{ 16 });
			
			for(Tuple& t : buffer) {
				t.offset = 0.0;
				t.delay = maxDispersion;
				t.time = Timestamp{};
			}
		}
		

		void Update(const Protocol::TimeSync & timeSync) {
			const Timestamp t0{ ConvertUInt64ToTimestamp(timeSync.client_req_transmission()) };
			const Timestamp t1{ ConvertUInt64ToTimestamp(timeSync.server_req_reception()) };
			const Timestamp t2{ ConvertUInt64ToTimestamp(timeSync.server_resp_transmission()) };
			const Timestamp t3{ ConvertUInt64ToTimestamp(timeSync.client_resp_reception()) };

			Duration delta = (t3 - t0) - (t2 - t1);
			Duration theta = ((t1 - t0) + (t2 - t3)) / 2;

			double offset = DurationToDouble(theta);
			double delay = std::max(DurationToDouble(delta), PRECISION);

			//Log::Debug("Delay: {0} Offset: {1}", delay, offset);
			
			// shift 1 out regardless of what happens next
			std::rotate(std::rbegin(buffer), std::rbegin(buffer) + 1, std::rend(buffer));
			buffer[0].delay = delay;
			buffer[0].offset = offset;
			buffer[0].time = t3;

			// filter step could end with an upgraded offset / delay
			ClockFilter();
		}
		
		
	};

	class ClientSession : public ClientSessionBase {
		boost::asio::io_context & ioContext;
		UdpResolver resolver;
		Ref<NetcodeService> service;
		Ref<ConnectionBase> connection;
		NtpClockFilter clockFilter;
		Enum<NatType> natType;
		std::string queryValueAddress;
		std::string queryValuePort;
		/*
		void SendLoginRequest() {
			Protocol::Header h;
			h.set_sequence(2);
			h.set_type(Protocol::MessageType::CONNECT_REQUEST);
			Protocol::ConnectRequest * connReq = h.mutable_connect_request();
			connReq->set_type(Protocol::ConnectType::DIRECT);
			Protocol::Endpoint * privateEndpoint = connReq->mutable_private_endpoint();
			auto ep = service->GetPrivateEndpoint();
			privateEndpoint->set_addr(ep.address().to_string());
			privateEndpoint->set_port(ep.port());

			connection->state = ConnectionState::AUTHENTICATING;
			Ref<TransmissionHandleBase> handle = service->Send(std::move(h), connection->endpoint);
			handle->ContinueWith([this](const MessageTransmissionResult & tr, const UdpEndpoint & ep) -> void {
				if(tr.result != TransmissionResult::SUCCESS) {
					Log::Error("LoginRequest: {0}", tr.errorIfAny.message());
				}
			});
		}*/
		
		void StartConnection() {
			if(connection == nullptr) {
				Log::Error("No connection was set");
				return;
			}

			connection->state = ConnectionState::CONNECTING;

			// start connection
		}
	public:
		ClientSession(boost::asio::io_context & ioc) : ioContext{ ioc }, resolver{ ioc } {
			
		}

		void OnHostnameResolved(const UdpResolver::results_type & results) {
			UdpSocket sock{ ioContext };

			connection->state = ConnectionState::CONNECTING;
			connection->endpoint = *(results.begin());

			ErrorCode ec;
			sock.open(connection->endpoint.protocol(), ec);

			auto netInterfaces = GetCompatibleInterfaces(connection->endpoint.address());

			if(netInterfaces.empty()) {
				Log::Warn("No defaultable network interface found");
			} else {
				const Interface bestCandidate = netInterfaces.front();

				sock.bind(UdpEndpoint{
					bestCandidate.address,
					0
				}, ec);

				if(ec) {
					Log::Error("Failed to bind port");
					connection->state = ConnectionState::INTERNAL_ERROR;
					return;
				}
			}

			if(ec) {
				Log::Error("Failed to open port");
				connection->state = ConnectionState::INTERNAL_ERROR;
				return;
			}

			if(ec) {
				Log::Error("Failed to 'connect': {0}", ec.message());
				connection->state = ConnectionState::INTERNAL_ERROR;
				return;
			}
			
			if(!SetDontFragmentBit(sock)) {
				Log::Warn("Failed to set dont fragment bit");
			}


			service = std::make_shared<NetcodeService>(std::move(sock));

			Log::Debug("Service created");

			Sleep(100);

			StartConnection();
		}

		void StartHostnameResolution() {
			connection->state = ConnectionState::RESOLVING;
			resolver.async_resolve(queryValueAddress, queryValuePort, boost::asio::ip::resolver_base::address_configured,
				[this](const ErrorCode & ec, UdpResolver::results_type results) -> void {
				if(ec) {
					Log::Error("Address resolution failed");
				} else {
					OnHostnameResolved(std::move(results));
				}
			});
		}
		
		virtual ~ClientSession() = default;
		
		virtual void Start() override {
			Ref<ConnectionBase> c = std::make_shared<ConnectionBase>();
			c->state = ConnectionState::INACTIVE;

			Connect(std::move(c), "localhost", 8889);
		}

		virtual void Stop() override {

		}

		
		virtual void Connect(Ref<ConnectionBase> connectionHandle, std::string hostname, uint32_t port) {
			if(connectionHandle->state != ConnectionState::INACTIVE) {
				Log::Error("State must be inactive to start a new connection");
				return;
			}

			if(connection != nullptr) {
				if(connection->state > 0x1000u) {
					Log::Error("Current connection seems to be active or pending");
					return;
				}
			}

			std::swap(connectionHandle, connection);

			queryValueAddress = std::move(hostname);
			queryValuePort = std::to_string(port);
			StartHostnameResolution();
		}
	};

}
