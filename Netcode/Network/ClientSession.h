#pragma once 

#include <Netcode/ModulesConfig.h>
#include "GameSession.h"
#include "NetworkCommon.h"
#include <boost/asio.hpp>

#include <NetcodeFoundation/Enum.hpp>
#include "NetcodeProtocol/header.pb.h"

#include <Netcode/System/System.h>

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
	public:
		constexpr static double DurationToDouble(const Duration & d) {
			return std::chrono::duration<double, std::milli>(d).count();
		}

		constexpr static Duration DoubleToDuration(double d) {
			return std::chrono::duration_cast<Duration>(std::chrono::duration<double, std::milli>(d));
		}
		
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

	struct ClockSyncResult {
		double offset;
		double delay;
		ErrorCode errorCode;
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

		void Tick();
		
		void InitTick() {
			tickTimer.expires_after(connection->tickInterval.load(std::memory_order_acquire));
			tickTimer.async_wait([this](const ErrorCode & ec) -> void {
				if(ec) {
					Log::Error("Tick: {0}", ec.message());
					return;
				}

				Tick();

				if(connection != nullptr) {
					if(connection->state != ConnectionState::INACTIVE) {
						InitTick();
					}
				}
			});
		}

		CompletionToken<TrResult> StartPunchthrough();

		void SendConnectRequest(CompletionToken<ErrorCode> mainToken);
		
		void StartConnection(CompletionToken<ErrorCode> mainToken);

		void OnHostnameResolved(const UdpResolver::results_type & results, CompletionToken<ErrorCode> mainToken);

		CompletionToken<ErrorCode> StartHostnameResolution();
		
		CompletionToken<ErrorCode> DiscoverPathMtu();

		CompletionToken<DtlsConnectResult> StartDtlsConnection();
		
	public:

		
		ClientSession(boost::asio::io_context & ioc) : ioContext{ ioc }, resolver{ ioc }, tickTimer{ ioc } {
		}

		virtual boost::asio::io_context & GetIOContext() override {
			return ioContext;
		}

		Ref<NetcodeService> GetService() const {
			return service;
		}
		
		virtual ~ClientSession() = default;
		
		void SendDebugFragmentedMessage();

		virtual void Start() override {
			
		}

		virtual void Stop() override {

		}

		virtual void CloseService();
		
		virtual CompletionToken<ErrorCode> Connect(Ref<ConnectionBase> connectionHandle, std::string hostname, uint32_t port) {
			if(connectionHandle->state != ConnectionState::INACTIVE) {
				throw UndefinedBehaviourException{ "connectionHandle must be INACTIVE" };
			}

			if(connection != nullptr) {
				if(connection->state > 0x1000u) {
					throw UndefinedBehaviourException{ "Current connection seems to be active or pending" };
				}
			}

			CloseService();

			std::swap(connectionHandle, connection);
			
			queryValueAddress = std::move(hostname);
			queryValuePort = std::to_string(port);
			return StartHostnameResolution();
		}
	};

}
