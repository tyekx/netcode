#include "FragmentStorage.h"
#include <Netcode/Utility.h>

namespace Netcode::Network {

	void FragmentStorage::TryReassemble(FSItem * p) {
		uint32_t dataSize = 0;
		if(FragmentsAreConsistent(p, &dataSize)) {
			uint32_t requiredSpace = Utility::Align<uint32_t, 512u>(dataSize + 512u);

			Ref<NetAllocator> dstAllocator;

			if(p->linkCount != 1) {
				dstAllocator = std::make_shared<NetAllocator>(nullptr, requiredSpace);
			} else {
				dstAllocator = p->allocator;
			}

			MutableArrayView<uint8_t> reassembledBinary{ dstAllocator->MakeArray<uint8_t>(dataSize), dataSize };

			uint32_t dstOffset = 0;

			for(FSItem * iter = p; iter != nullptr; iter = iter->orderedNext) {
				const uint32_t sourceFragSize = iter->fragment->contentSize;

				memcpy(reassembledBinary.Data() + dstOffset, iter->fragment->content + NC_HEADER_SIZE, sourceFragSize - NC_HEADER_SIZE);

				dstOffset += sourceFragSize - NC_HEADER_SIZE;
			}

			//TODO: handle reassembled message

			/*
			Protocol::Update * update = dstAllocator->MakeProto<Protocol::Update>();
			google::protobuf::io::ArrayInputStream ais{ reassembledBinary.Data(), static_cast<int32_t>(reassembledBinary.Size()) };

			if(update->ParseFromZeroCopyStream(&ais)) {
			Ref<ConnectionBase> conn = connectionStorage->GetConnectionByEndpoint(p->fragment->packet->endpoint);

			if(conn != nullptr) {
			GameMessage msg;
			msg.allocator = std::move(dstAllocator);
			msg.update = update;
			conn->sharedQueue.Received(std::move(msg));
			}
			}*/
		}

		FSItem * it = p;
		while(it != nullptr) {
			FSItem * tmp = it->orderedNext;
			MarkDeletable(it);
			it = tmp;
		}
	}
	
}
