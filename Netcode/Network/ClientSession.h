#pragma once 

#include <Netcode/ModulesConfig.h>
#include "GameSession.h"
#include "NetworkCommon.h"
#include <boost/asio.hpp>

#include "NetcodeProtocol/header.pb.h"

namespace Netcode::Network {

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
		std::string queryValueAddress;
		std::string queryValuePort;

	public:
		ClientSession(boost::asio::io_context & ioc) : ioContext{ ioc }, resolver{ ioc } {
			
		}

		void OnAddrResolved(const UdpResolver::results_type & results) {
			UdpSocket sock{ ioContext };

			UdpEndpoint remoteEndpoint = *(results.begin());

			ErrorCode ec;
			sock.open(remoteEndpoint.protocol(), ec);

			if(ec) {
				Log::Error("Failed to open port");
				return;
			}
			
			sock.connect(remoteEndpoint, ec);

			if(ec) {
				Log::Error("Failed to 'connect'");
				return;
			}

			service = std::make_shared<NetcodeService>(std::move(sock));
			service->Host();

			Log::Debug("Service created");

			Sleep(100);

			Protocol::Header h;
			h.set_sequence(1);
			h.set_type(Protocol::MessageType::CONNECT_PUNCHTHROUGH);
			
			service->Send(std::move(h), remoteEndpoint);
		}
		
		virtual ~ClientSession() = default;
		
		virtual void Start() override {
			queryValueAddress = "::1";
			queryValuePort = "8889";

			resolver.async_resolve(queryValueAddress, queryValuePort, boost::asio::ip::resolver_base::address_configured,
				[this](const ErrorCode& ec, UdpResolver::results_type results) -> void {
					if(ec) {
						Log::Error("Address resolution failed");
					} else {
						OnAddrResolved(std::move(results));
					}
				});
		}
		
		virtual void Stop() override {

		}
	};

}
