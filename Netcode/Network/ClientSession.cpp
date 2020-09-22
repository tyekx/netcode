#include "ClientSession.h"
#include "Macros.h"
#include <Netcode/Logger.h>
#include <Netcode/Config.h>
#include <Netcode/Utility.h>
#include <NetcodeProtocol/header.pb.h>
#include <google/protobuf/io/coded_stream.h>

namespace Netcode::Network {

	struct PmtuDiscovery {
		Ref<NetcodeUdpSocket> udpSocket;
		Ref<TransmissionHandleBase> activeHandle;
		uint16_t lastMtuAttempt;

		// some common mtu values
		constexpr static uint16_t POSSIBLE_MTU_VALUES[] = {
			68, // ipv4 min
			576, // ipv6 min
			1006,
			1280,
			1492,
			1500, // most common
			2002,
			4352,
			8166
		};

		// discovery strategy is dirty.
		void InitDiscovery(uint16_t mtuAttempt) {
			Protocol::Header header;
			header.set_type(Protocol::MessageType::PMTU_DISCOVERY);

			uint16_t target = POSSIBLE_MTU_VALUES[0];
			for(uint16_t value : POSSIBLE_MTU_VALUES) {
				if(value > mtuAttempt) {
					break;
				}
				target = value;
			}
			lastMtuAttempt = target;
			header.set_mtu_probe_value(static_cast<uint32_t>(lastMtuAttempt));
			//activeHandle = udpSocket->Send(std::move(header), );
		}
		
	public:
		PmtuDiscovery(Ref<NetcodeUdpSocket> sock) : udpSocket{ std::move(sock) }, activeHandle{}, lastMtuAttempt{ 1500 } {
			//stream->GetSocket()...
			lastMtuAttempt = 1500;
			InitDiscovery(lastMtuAttempt);
		}
		
		bool IsDone() {
			if(!activeHandle->IsComplete()) {
				return false;
			}

			MessageTransmissionResult result = activeHandle->GetResult();

			// we came from higher values, so the first success means we are done
			if(result.result == TransmissionResult::SUCCESS) {
				return true;
			}

			if(result.result == TransmissionResult::TIMEOUT) {
				// are there even any next?
				if(POSSIBLE_MTU_VALUES[0] == lastMtuAttempt) {
					return true;
				} else {
					InitDiscovery(lastMtuAttempt - 1);
					return false;
				}
			}

			if(result.result == TransmissionResult::ERROR_WHILE_SENDING) {
				// maybe we are just unreachable?
				
			}
		}

	};
	
}
