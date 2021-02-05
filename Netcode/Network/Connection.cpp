#include "Connection.h"
#include "NetworkErrorCode.h"

namespace Netcode::Network {
	void PendingTokenStorage::Ack(uint32_t sequence, const UdpEndpoint & sender, AckClassification ackClass) {
		ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

		PendingTokenNode * prev = nullptr;

		for(PendingTokenNode * iter = head; iter != nullptr; iter = iter->next) {
			if(iter->packet->GetEndpoint() == sender && iter->packet->GetSequence() == sequence && iter->ackClass == ackClass) {
				if(prev == nullptr) {
					head = iter->next;
				} else {
					prev->next = iter->next;
				}

				TrResult tr;
				iter->token->Set(TrResult{ make_error_code(NetworkErrc::SUCCESS), iter->packet->GetSize() });
				iter->timer->cancel();
				CompletionToken<TrResult> tmpToken = std::move(iter->token);
				return;
			}

			prev = iter;
		}
	}
}
