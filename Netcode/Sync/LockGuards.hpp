#pragma once


namespace Netcode {

	/*
	 * "ExclusivelyLockable" concept:
	 * required functions:
		AcquireExclusive()
		ReleaseExclusive()
	 */
	template<typename T>
	class ScopedExclusiveLock {
		T & lock;
	public:
		ScopedExclusiveLock(T & lock) : lock{ lock } {
			lock.AcquireExclusive();
		}
		~ScopedExclusiveLock() {
			lock.ReleaseExclusive();
		}

		ScopedExclusiveLock(ScopedExclusiveLock &&) = delete;
		ScopedExclusiveLock(const ScopedExclusiveLock &) = delete;
		ScopedExclusiveLock & operator=(ScopedExclusiveLock &&) = delete;
		ScopedExclusiveLock & operator=(const ScopedExclusiveLock &) = delete;
	};

	/*
	 * "SharedlyLockable" concept:
	 * required functions:
		AcquireShared()
		ReleaseShared()
	 */
	template<typename T>
	class ScopedSharedLock {
		T & lock;
	public:
		ScopedSharedLock(T & lock) : lock{ lock } {
			lock.AcquireShared();
		}
		~ScopedSharedLock() {
			lock.ReleaseShared();
		}

		ScopedSharedLock(ScopedSharedLock &&) = delete;
		ScopedSharedLock(const ScopedSharedLock &) = delete;
		ScopedSharedLock & operator=(ScopedSharedLock &&) = delete;
		ScopedSharedLock & operator=(const ScopedSharedLock &) = delete;
	};
	
}
