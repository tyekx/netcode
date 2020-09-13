#pragma once

#include <NetcodeFoundation/Macros.h>
#include <NetcodeFoundation/Platform.h>

#if defined(NETCODE_OS_WINDOWS)
#include <synchapi.h>
#endif


namespace Netcode {

	class SlimReadWriteLock {
		SRWLOCK lock;
	public:
		~SlimReadWriteLock() = default;
		SlimReadWriteLock() : lock{} {
			InitializeSRWLock(&lock);
		}

		NETCODE_CONSTRUCTORS_DELETE_COPY(SlimReadWriteLock);
		NETCODE_CONSTRUCTORS_DELETE_MOVE(SlimReadWriteLock);

		void AcquireExclusive() {
			AcquireSRWLockExclusive(&lock);
		}

		void ReleaseExclusive() {
			ReleaseSRWLockExclusive(&lock);
		}

		void AcquireShared() {
			AcquireSRWLockShared(&lock);
		}

		void ReleaseShared() {
			ReleaseSRWLockShared(&lock);
		}
	};

}
