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
		double offsetApprox;
		double delayApprox;
		
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

			delayApprox = f0->delay;
			offsetApprox = f0->offset;
			
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
		
		double GetDelay() const {
			return delayApprox;
		}
		
		double GetOffset() const {
			return offsetApprox;
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
		WaitableTimer tickTimer;
		Ref<NetcodeService> service;
		Ref<ConnectionBase> connection;
		NtpClockFilter clockFilter;
		Enum<NatType> natType;
		std::string queryValueAddress;
		std::string queryValuePort;

		void SendConnectRequest(CompletionToken<ErrorCode> mainToken);

		void Tick();
		
		void InitTick() {
			tickTimer.expires_from_now(std::chrono::milliseconds(500));
			tickTimer.async_wait([this](const ErrorCode & ec) -> void {
				if(ec) {
					Log::Error("Tick: {0}", ec.message());
					return;
				}

				Tick();
				
				InitTick();
			});
		}
		
		void StartConnection(CompletionToken<ErrorCode> mainToken) {
			if(connection == nullptr) {
				Log::Error("No connection was set");
				mainToken->Set(make_error_code(Error::BAD_API_CALL));
				return;
			}

			connection->state = ConnectionState::CONNECTING;

			Ref<NetAllocator> alloc = service->MakeSmallAllocator();
			Protocol::Header* header = alloc->MakeProto<Protocol::Header>();
			header->set_type(Protocol::MessageType::CONNECT_PUNCHTHROUGH);

			service->Send(std::move(alloc), connection.get(), header)
				   ->Then([this, mainToken](TrResult tr) mutable -> void {
				if(tr.errorCode) {
					mainToken->Set(tr.errorCode);
				} else {
					SendConnectRequest(std::move(mainToken));
				}
			});
		}

		void OnHostnameResolved(const UdpResolver::results_type & results, CompletionToken<ErrorCode> mainToken) {
			if(results.empty()) {
				mainToken->Set(make_error_code(Error::ADDRESS_RESOLUTION_FAILED));
				return;
			}
			
			UdpSocket sock{ ioContext };

			connection->state = ConnectionState::CONNECTING;
			connection->endpoint = *(results.begin());
			connection->localSequence = 1;

			ErrorCode ec;
			sock.open(connection->endpoint.protocol(), ec);

			auto netInterfaces = GetCompatibleInterfaces(connection->endpoint.address());

			uint32_t linkLocalMtu = 1280;

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
					mainToken->Set(make_error_code(Error::SOCK_ERR));
					connection->state = ConnectionState::INACTIVE;
					return;
				}

				linkLocalMtu = bestCandidate.mtu;

#if defined(NETCODE_DEBUG)
				try {
					uint32_t fakeMtu = Config::Get<uint32_t>(L"network.fakeMtu:u32");
					if(fakeMtu > 0) {
						linkLocalMtu = std::min(linkLocalMtu, fakeMtu);
					}
				} catch(OutOfRangeException & e) { }
#endif
			}

			if(ec) {
				Log::Error("Failed to open port");
				mainToken->Set(make_error_code(Error::SOCK_ERR));
				connection->state = ConnectionState::INACTIVE;
				return;
			}

			if(ec) {
				Log::Error("Failed to 'connect': {0}", ec.message());
				mainToken->Set(make_error_code(Error::SOCK_ERR));
				connection->state = ConnectionState::INACTIVE;
				return;
			}
			
			if(!SetDontFragmentBit(sock)) {
				Log::Error("Failed to set dont fragment bit");
				mainToken->Set(make_error_code(Error::SOCK_ERR));
				connection->state = ConnectionState::INACTIVE;
				return;
			}
			
			service = std::make_shared<NetcodeService>(ioContext, std::move(sock), static_cast<uint16_t>(linkLocalMtu));
			service->Host();

			Log::Debug("Service created");

			Sleep(100);

			StartConnection(std::move(mainToken));

			InitTick();
		}

		CompletionToken<ErrorCode> StartHostnameResolution() {
			CompletionToken<ErrorCode> ct = std::make_shared<CompletionTokenType<ErrorCode>>(&ioContext);
			connection->state = ConnectionState::RESOLVING;
			resolver.async_resolve(queryValueAddress, queryValuePort, boost::asio::ip::resolver_base::address_configured,
				[this, ct](const ErrorCode & ec, UdpResolver::results_type results) mutable -> void {
				if(ec) {
					Log::Error("Address resolution failed");
				} else {
					OnHostnameResolved(std::move(results), std::move(ct));
				}
			});
			return ct;
		}
		
		CompletionToken<ErrorCode> DiscoverPathMtu();

	public:
		ClientSession(boost::asio::io_context & ioc) : ioContext{ ioc }, resolver{ ioc }, tickTimer{ ioc } {

		}

		Ref<NetcodeService> GetService() const {
			return service;
		}
		
		virtual ~ClientSession() = default;
		
		void SendDebugFragmentedMessage() {
			Ref<NetAllocator> alloc = service->MakeLargeAllocator();
			Protocol::Update * update = alloc->MakeProto<Protocol::Update>();

			Protocol::ClientUpdate * cu = update->mutable_client_update();
			Protocol::Player * ps = cu->mutable_player_state();

			char * data = alloc->MakeArray<char>(8000);
			memset(data, 'A', 8000);

			ps->set_replication_data(data, 8000);

			service->Send(std::move(alloc), connection.get(), update)->Then([this](const TrResult & tr) -> void {
				Log::Debug("Send: {0}", tr.errorCode.message());
				//SendDebugFragmentedMessage();
			});
		}

		virtual void Start() override {
			Ref<ConnectionBase> c = std::make_shared<ConnectionBase>();
			c->state = ConnectionState::INACTIVE;

			Connect(std::move(c), "localhost", 8889)->Then([this](const ErrorCode & ec) -> void {
				Log::Debug("Connect: {0}", ec.message());

				if(ec) {
					Log::Error("Failed to connect: {0}", ec.message());
					connection->state = ConnectionState::INACTIVE;
					return;
				}

				connection->state = ConnectionState::SYNCHRONIZING;

				Synchronize()->Then([this](const ErrorCode & ec) -> void {
					if(ec) {
						Log::Error("Failed to synchronize: {0}", ec.message());
						connection->state = ConnectionState::INACTIVE;
						return;
					}

					connection->state = ConnectionState::ESTABLISHED;

					DiscoverPathMtu()->Then([this](const ErrorCode & ec)-> void {
						Log::Debug("Pmtu discovery: {0}", ec.message());
					});
				});
			});
		}

		virtual void Stop() override {

		}

		virtual CompletionToken<ErrorCode> Synchronize();
		
		virtual CompletionToken<ErrorCode> Connect(Ref<ConnectionBase> connectionHandle, std::string hostname, uint32_t port) {
			if(connectionHandle->state != ConnectionState::INACTIVE) {
				throw UndefinedBehaviourException{ "connectionHandle must be INACTIVE" };
			}

			if(connection != nullptr) {
				if(connection->state > 0x1000u) {
					throw UndefinedBehaviourException{ "Current connection seems to be active or pending" };
				}
			}

			std::swap(connectionHandle, connection);
			
			queryValueAddress = std::move(hostname);
			queryValuePort = std::to_string(port);
			return StartHostnameResolution();
		}
	};

}
