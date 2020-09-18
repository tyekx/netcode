#pragma once 

#include "../ModulesConfig.h"
#include "GameSession.h"
#include "NetworkCommon.h"
#include <boost/asio.hpp>

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
		Ref<PacketStorage<UdpPacket>> packetStorage;
		MessageQueue<Protocol::ServerUpdate> gameQueue;
		boost::asio::deadline_timer timer;
		UdpResolver resolver;
		Ref<Connection> connection;
		NtpClockFilter clockFilter;
		std::string queryValueAddress;
		std::string queryValuePort;
		
		void SetError(const boost::system::error_code & ec);

		void Tick();

		void OnTimerExpired(const boost::system::error_code & ec);

		void InitTimeSyncStep();
		
		void InitTimer();
		void InitRead();
		void OnRead(Ref<UdpPacket> packet);
		void OnMessageSent(const ErrorCode & ec, size_t size);

	public:
		ClientSession(boost::asio::io_context & ioc);
		virtual ~ClientSession() = default;
		virtual void Start() override;
		
		virtual void Stop() override {

		}

		virtual void SwapBuffers(std::vector<Protocol::ServerUpdate> & game) override {
			
		}
		
		virtual void Update(Protocol::ClientUpdate message) override;
		
		virtual Ref<Connection> Connect(std::string address, uint16_t port, std::string nonce) override;
		
		virtual void Disconnect() override {
			
		}
	};

}
