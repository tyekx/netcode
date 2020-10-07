#include "Connection.h"

namespace Netcode::Network {


	CompletionToken<TrResult> NetcodeService::Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> ct, Protocol::Header * header, const UdpEndpoint & endpoint, MtuValue pmtu, ResendArgs args) {
		if(header->type() == Protocol::MessageType::GAME) {
			throw UndefinedBehaviourException{ "bad API call" };
		}

		const uint32_t serializedHeaderSize = static_cast<uint32_t>(header->ByteSizeLong());
		const uint32_t totalHeaderSize = serializedHeaderSize + 8;
		uint32_t binarySize = totalHeaderSize;
		uint32_t mtuProbeDataSize = 0;

		if(header->type() == Protocol::MessageType::PMTU_DISCOVERY) {
			uint32_t mtuProbeValue = pmtu.GetMaxPayloadSize(endpoint.address());

			// adding +8 here helps catching mtuProbeValue = 0 instead of making it underflow by -8.
			if(mtuProbeValue < (totalHeaderSize + 8)) {
				throw UndefinedBehaviourException{ "MTU probe value too small" };
			}

			mtuProbeDataSize = mtuProbeValue - totalHeaderSize - 8;
			binarySize += mtuProbeDataSize;
		}

		UdpPacket * pkt = allocator->MakeUdpPacket(binarySize);
		pkt->SetEndpoint(endpoint);
		pkt->SetSequence(header->sequence());
		pkt->SetTimestamp(SystemClock::LocalNow());
		pkt->SetMtu(header->mtu_probe_value());

		google::protobuf::io::ArrayOutputStream outStream{ pkt->GetData(), static_cast<int32_t>(pkt->GetCapacity()) };
		google::protobuf::io::CodedOutputStream codedOutStream{ &outStream };

		codedOutStream.WriteLittleEndian32(static_cast<uint32_t>(serializedHeaderSize));
		if(!header->SerializeToCodedStream(&codedOutStream)) {
			Log::Warn("Failed to serialize header");
		}

		if(header->type() == Protocol::MessageType::PMTU_DISCOVERY) {
			pkt->SetFragmentData(FragmentData{ 0, 1, static_cast<uint16_t>(mtuProbeDataSize) });
			codedOutStream.WriteLittleEndian32(pkt->GetFragmentData().Pack());

			uint8_t * rawBuffer = codedOutStream.GetDirectBufferForNBytesAndAdvance(mtuProbeDataSize);

			if(rawBuffer == nullptr) {
				throw OutOfRangeException{ "MTU probe size is too big" };
			}
			// 60 is palindrom in binary
			memset(rawBuffer, 60, mtuProbeDataSize);
		} else {
			codedOutStream.WriteLittleEndian32(FragmentData{}.Pack());
		}
		pkt->SetSize(binarySize);

		if((header->type() & 0x1) == 0x1) {
			WaitableTimer * timer = allocator->Make<WaitableTimer>(ioContext);
			PendingTokenNode * node = allocator->Make<PendingTokenNode>(ct, timer, pkt);

			Ref<ResendContext<NetcodeSocketType>> rc = allocator->MakeShared<ResendContext<NetcodeSocketType>>(
				&pendingTokenStorage, &socket, ct, pkt, timer, args.resendInterval, args.maxAttempts);

			pendingTokenStorage.AddNode(node);

			rc->Attempt();
		} else {
			socket.Send(pkt->GetConstBuffer(), pkt->GetEndpoint(), [ct](const ErrorCode & ec, size_t s) -> void {
				ct->Set(TrResult{ ec, s });
			});
		}

		return ct;
	}


	CompletionToken<TrResult> NetcodeService::Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> ct, Protocol::Update * update, const UdpEndpoint & endpoint, MtuValue pmtu, uint32_t seq) {
		Protocol::Header * header = allocator->MakeProto<Protocol::Header>();

		header->set_sequence(seq);
		header->set_type(Protocol::MessageType::GAME);

		const uint32_t serializedDataSize = static_cast<uint32_t>(update->ByteSizeLong());
		const uint32_t serializedHeaderSize = static_cast<uint32_t>(header->ByteSizeLong());
		const uint32_t totalHeaderSize = serializedHeaderSize + 2 * sizeof(google::protobuf::uint32);
		const uint32_t localMtu = pmtu.GetMaxPayloadSize(endpoint.address()) - 8; // - udp header
		const uint32_t maxPayloadSize = localMtu - totalHeaderSize;
		const uint32_t numFragments = serializedDataSize / maxPayloadSize + (std::min(serializedDataSize % maxPayloadSize, 1u));

		const uint32_t binaryDataSize = numFragments * (localMtu);

		if(binaryDataSize > UdpPacket::IPV6_MAX_SIZE) {
			throw OutOfRangeException{ "Network packet too big" };
		}

		UdpPacket * pkt = allocator->MakeUdpPacket(binaryDataSize);
		pkt->SetEndpoint(endpoint);
		pkt->SetSequence(seq);
		pkt->SetMtu(localMtu);
		pkt->SetTimestamp(SystemClock::LocalNow());
		pkt->SetSize(numFragments * totalHeaderSize + serializedDataSize);

		google::protobuf::io::ArrayOutputStream outStream{ pkt->GetData(), static_cast<int32_t>(pkt->GetCapacity()) };
		google::protobuf::io::CodedOutputStream codedOutStream{ &outStream };

		uint32_t remainingSize = serializedDataSize;
		const uint32_t headerStart = 0;
		const uint32_t dataStart = numFragments * totalHeaderSize;

		for(uint32_t i = 0; i < numFragments; i++) {
			codedOutStream.WriteLittleEndian32(serializedHeaderSize);
			if(!header->SerializeToCodedStream(&codedOutStream)) {
				throw UndefinedBehaviourException{ "Failed to serialize header while fragmenting" };
			}

			FragmentData fd;
			fd.fragmentCount = static_cast<uint8_t>(numFragments);
			fd.fragmentIndex = static_cast<uint8_t>(i);
			fd.sizeInBytes = static_cast<uint16_t>(std::min(remainingSize, maxPayloadSize));
			codedOutStream.WriteLittleEndian32(fd.Pack());

			remainingSize -= fd.sizeInBytes;
		}

		if(!update->SerializePartialToCodedStream(&codedOutStream)) {
			throw UndefinedBehaviourException{ "Failed to serialize update while fragmenting" };
		}

		Ref<FragmentationContext<NetcodeSocketType>> fragCtx =
			allocator->MakeShared<FragmentationContext<NetcodeSocketType>>(
				ct, &socket, pkt, maxPayloadSize, dataStart, headerStart, serializedDataSize, totalHeaderSize);

		fragCtx->SendFragment();

		return ct;
	}

}
