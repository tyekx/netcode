#include <algorithm>

#include "NetworkCommon.h"
#include "../Logger.h"

namespace Netcode::Network {


	PacketStorage::StorageType PacketStorage::AllocateStorage() {
		// make_shared<uint8_t[]> wouldnt work here, so make_unique it is
		return std::shared_ptr<uint8_t[]>(std::make_unique<uint8_t[]>(PACKET_STORAGE_SIZE));
	}

	PacketStorage::PacketStorage(uint32_t preallocatedBuffers) : availableBuffers{} {
		availableBuffers.reserve(preallocatedBuffers * 2);

		for(uint32_t i = 0; i < preallocatedBuffers; ++i) {
			auto ptr = AllocateStorage();
			availableBuffers.emplace_back(std::move(ptr));
		}
	}

	void PacketStorage::ReturnBuffer(StorageType storage) {
		availableBuffers.emplace_back(std::move(storage));
	}

	void PacketStorage::ReturnBuffer(std::vector<StorageType> buffers)
	{
		std::move(std::begin(buffers), std::end(buffers), std::back_inserter(availableBuffers));
	}

	PacketStorage::StorageType PacketStorage::GetBuffer() {
		if(availableBuffers.empty()) {
			return AllocateStorage();
		} else {
			auto ptr = availableBuffers.back();
			availableBuffers.pop_back();
			return ptr;
		}
	}

	NetworkContext::~NetworkContext() {
		Stop();
	}

	NetworkContext::NetworkContext() : ioc{}, work{ ioc }, workers{} {
	
	}

	void NetworkContext::Start(uint8_t numThreads) {
		if(!workers.empty()) {
			return;
		}

		numThreads = std::clamp(numThreads, static_cast<uint8_t>(1), static_cast<uint8_t>(4));

		workers.reserve(numThreads);

		for(uint8_t i = 0; i < numThreads; ++i) {
			workers.emplace_back([this]() -> void {
				ioc.run();
			});
		}
	}

	void NetworkContext::Stop() {
		ioc.stop();

		for(auto & thread : workers) {
			thread.join();
		}
	}

	boost::asio::io_context & NetworkContext::GetImpl() {
		return ioc;
	}

	void ControlPacketStorage::CheckTimeouts(uint64_t durationMs) {
		auto now = std::chrono::steady_clock::now();

		auto it = storage.begin();

		do {
			for(; it != storage.end(); ++it) {
				// on an empty entry, we erase it
				if(it->second.empty()) {
					storage.erase(it);
					it = storage.begin();
					break;
				}

				Item & item = it->second.front();
				auto timestamp = item.first_sent_at;


				if(timestamp.time_since_epoch() == std::chrono::steady_clock::duration::zero()) {
					// was not sent yet
					continue;
				}

				if(std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp).count() > durationMs) {
					item.promise.set_value(boost::asio::error::make_error_code(boost::asio::error::timed_out));
					Log::Info("[Network] Control packet timed out, no ACK was received");
					it->second.pop_front();
				}
			}
		} while(it != storage.end());
	}

	ErrorCode ControlPacketStorage::Acknowledge(const udp_endpoint_t & source, int32_t ackId) {
		auto it = storage.find(source);

		if(it == storage.end() || it->second.empty()) {
			return boost::asio::error::make_error_code(boost::asio::error::not_found);
		}

		Item & item = it->second.front();

		// acking a packet that was not sent yet is an error
		if(item.packetId < ackId) {
			return Errc::make_error_code(Errc::protocol_error);
		}

		if(item.packetId == ackId) {
			it->second.front().promise.set_value(Errc::make_error_code(Errc::success));
			it->second.pop_front();
			Log::Debug("[Network] ACK OK: {0}", ackId);
		} // else: possibly resent ACK that got reordered

		return Errc::make_error_code(Errc::success);
	}

	std::future<ErrorCode> ControlPacketStorage::Push(int32_t id, UdpPacket packet) {
		return storage[packet.endpoint].emplace_back(id, std::move(packet)).promise.get_future();
	}

}
