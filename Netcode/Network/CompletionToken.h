#pragma once

#include "NetworkDecl.h"
#include <Netcode/HandleDecl.h>
#include <Netcode/PlacedFunction.hpp>
#include <atomic>
#include <boost/asio.hpp>

namespace Netcode::Network {

	/*
	 * LockFree completion token
	 * can facilitate a single callback function and a single result
	 * internals does not allocate
	 */
	template<typename T>
	class CompletionTokenType : public std::enable_shared_from_this<CompletionTokenType<T>> {
		std::aligned_storage_t<sizeof(T)> storage;
		PlacedFunction<56, void(const T &)> callback;
		boost::asio::io_context * ioc;
		/*
		 *  - state only increases with CAS operations
		 *  - 1: callback write started flag
		 *  - 2: result write started flag
		 *  - 4: callback saved
		 *  - 8: result saved
		 *  - 15: every neccessary flag set, ready to invoke
		 *  - 31: all done
		 */
		std::atomic_uint32_t state;

		using Base = std::enable_shared_from_this<CompletionTokenType<T>>;

		void InvokeCallbackUnsafe() {
			callback(*reinterpret_cast<const T *>(std::addressof(storage)));
		}

		void TryInvoke() {
			uint32_t expectedValue = 0xF;
			if(state.compare_exchange_strong(expectedValue, 0x1F, std::memory_order_release)) {
				boost::asio::post(*ioc, [this, lifetime = Base::shared_from_this()]() -> void {
					InvokeCallbackUnsafe();
				});
			}
		}

		bool TrySetFlag(uint32_t flag) {
			for(;;) {
				uint32_t currentState = state.load(std::memory_order_acquire);

				if((currentState & flag) == flag) {
					return false;
				}

				const uint32_t desiredState = state | flag;

				if(state.compare_exchange_strong(currentState, desiredState, std::memory_order_release)) {
					return true;
				}
			}
		}

	public:
		CompletionTokenType(boost::asio::io_context * ioc = nullptr) : storage{}, callback{}, ioc{ ioc }, state{ 0 } {

		}

		~CompletionTokenType() {
			if((state & 0xA) == 0xA) {
				reinterpret_cast<T *>(std::addressof(storage))->~T();
			}
		}

		template<typename Functor>
		CompletionTokenType(boost::asio::io_context * ioc, Functor && f) :
			storage{},
			callback{ std::forward<Functor>(f) },
			ioc{ ioc },
			state{ 0x5 } {

		}

		CompletionTokenType(const CompletionTokenType &) = delete;
		CompletionTokenType(CompletionTokenType &&) noexcept = delete;
		CompletionTokenType & operator=(const CompletionTokenType &) = delete;
		CompletionTokenType & operator=(CompletionTokenType &&) noexcept = delete;

		[[nodiscard]]
		bool IsCompleted() const {
			return (state.load(std::memory_order_acquire) & 0xA) == 0xA;
		}

		[[nodiscard]]
		bool HasCallback() const {
			return (state.load(std::memory_order_acquire) & 0x5) == 0x5;
		}

		template<typename Functor>
		bool Then(Functor f) {
			if(TrySetFlag(0x1)) {
				callback = std::move(f);
				if(TrySetFlag(0x4)) {
					TryInvoke();
				}
				return true;
			}
			return false;
		}

		bool Set(T obj) {
			if(TrySetFlag(0x2)) {
				new (std::addressof(storage)) T{ std::move(obj) };
				if(TrySetFlag(0x8)) {
					TryInvoke();
				}
				return true;
			}
			return false;
		}
	};

}
