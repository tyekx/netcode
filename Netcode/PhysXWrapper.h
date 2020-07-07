#pragma once

#include "Modules.h"
#include <physx/PxPhysicsAPI.h>
#include <vector>
#include <memory>

#define PX_RELEASE(ptr) if(ptr) { ptr->release(); ptr = nullptr; }

namespace Netcode {

	template<typename T>
	class PxPtr {
		T * ptr;


	public:
		void Reset(T * p = nullptr) {
			if(ptr) {
				ptr->release();
			}

			ptr = p;
		}

		~PxPtr() noexcept {
			try {
				Reset();
			} catch(...) { }
		}

		PxPtr() : ptr{ nullptr } { }
		PxPtr(T * p) : ptr{ p } { }
		PxPtr(PxPtr<T> && rhs) : ptr{ std::move(rhs.ptr) } { }
		PxPtr & operator=(const PxPtr<T> & rhs) = delete;
		PxPtr & operator=(PxPtr<T> rhs) noexcept {
			std::swap(ptr, rhs.ptr);
			return *this;
		}

		PxPtr & operator=(T * p) {
			Reset(p);
			return *this;
		}

		constexpr auto * operator->() {
			return ptr;
		}

		constexpr auto * operator->() const {
			return ptr;
		}

		T * Get() {
			return ptr;
		}

		T & operator*() {
			return *ptr;
		}

		bool operator!=(std::nullptr_t) const {
			return ptr != nullptr;
		}

	};

}

namespace Netcode::Physics {

	//A wrapper class for the basic physx initialization and cleanup
	class PhysX {
	public:
		physx::PxDefaultAllocator allocator;
		physx::PxDefaultErrorCallback errorCallback;
		PxPtr<physx::PxFoundation> foundation;
		PxPtr<physx::PxPhysics> physics;
		PxPtr<physx::PxPvd> debugger;
		PxPtr<physx::PxDefaultCpuDispatcher> dispatcher;
		PxPtr<physx::PxCooking> cooking;

		void UpdateDebugCamera(const Float3 & pos, const Float3 & up, const Float3 & lookAt);

		void CreateResources();

		void ReleaseResources();


	};

}
