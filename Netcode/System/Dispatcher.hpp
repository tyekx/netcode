#pragma once

#include <Netcode/PlacedFunction.hpp>
#include <Netcode/Sync/SlimReadWriteLock.h>
#include <Netcode/Sync/LockGuards.hpp>
#include <vector>

namespace Netcode {

	class Dispatcher {

		using FnType = PlacedFunction<120, void()>;

		SlimReadWriteLock srwLock;
		std::vector<FnType> callbacks;
		
	public:

		NETCODE_CONSTRUCTORS_NO_COPY_NO_MOVE(Dispatcher);
		
		template<typename Callback>
		void Post(Callback && fn) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			callbacks.emplace_back(std::forward<Callback>(fn));
		}

		void Run() {
			if(callbacks.empty()) {
				return;
			}

			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			
			for(FnType& f: callbacks) {
				f();
			}
			
			callbacks.clear();
		}
		
	};
	
}
