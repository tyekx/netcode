#include <algorithm>

#include "NetworkCommon.h"
#include "../Logger.h"
#include "Macros.h"

namespace Netcode::Network {


	std::unique_ptr<uint8_t[]> PacketStorage::AllocateStorage() {
		return std::make_unique<uint8_t[]>(PACKET_STORAGE_SIZE);
	}

	PacketStorage::PacketStorage(uint32_t preallocatedBuffers) : srwLock{}, availableBuffers {} {
		availableBuffers.reserve(preallocatedBuffers * 2);

		for(uint32_t i = 0; i < preallocatedBuffers; ++i) {
			auto ptr = AllocateStorage();
			availableBuffers.emplace_back(std::move(ptr));
		}
	}

	Ref<uint8_t[]> PacketStorage::GetBuffer() {
		ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
		
		if(availableBuffers.empty()) {
			std::unique_ptr<uint8_t[]> b = AllocateStorage();
			return Ref<uint8_t[]>{ b.release(), GetDestructor() };
		}
		
		auto ptr = std::move(availableBuffers.back());
		availableBuffers.pop_back();
		return Ref<uint8_t[]>{ ptr.release(), GetDestructor() };
	}

	NetworkContext::~NetworkContext() {
		Stop();
	}

	NetworkContext::NetworkContext() : ioc{}, work{ std::make_unique<boost::asio::io_context::work>(ioc) }, workers{} {
	
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
		if(workers.empty()) {
			return;
		}

		if(work) {
			work.reset();
		}

		int32_t numThreads = static_cast<int32_t>(workers.size());

		ioc.stop();

		for(auto & thread : workers) {
			thread.join();
		}

		workers.clear();

		Log::Info("[Network] ({0}) I/O threads were joined successfully", numThreads);
	}

	boost::asio::io_context & NetworkContext::GetImpl() {
		return ioc;
	}

	ErrorCode Bind(const boost::asio::ip::address & selfAddr, udp_socket_t & udpSocket, uint32_t & port) {
		uint32_t portHint = port;
		port = std::numeric_limits<uint32_t>::max();

		if(portHint < 1024 || portHint > 49151) {
			portHint = (49151 - 1024) / 2;
		}

		ErrorCode ec;
		const uint32_t range = 49151 - 1024;
		const int32_t start = static_cast<int32_t>(portHint);
		int32_t sign = 1;

		udp_endpoint_t endpoint{ selfAddr, 0 };

		udpSocket.open(endpoint.protocol(), ec);

		if(ec) {
			return ec;
		}

		for(int32_t i = 0; i < range; ++i, sign = -sign) {
			uint32_t portToTest = static_cast<uint32_t>(start + sign * i / 2);

			if(portToTest < 1024 || portToTest > 49151) {
				continue;
			}

			endpoint.port(portToTest);

			udpSocket.bind(endpoint, ec);

			if(ec == boost::asio::error::bad_descriptor) { // expected error
				continue;
			}

			if(ec) { // unexpected error 
				return ec;
			}

			port = portToTest;

			return Errc::make_error_code(Errc::success);
		}

		return ec;
	}

}
