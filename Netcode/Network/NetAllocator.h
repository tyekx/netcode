#pragma once

#include <memory>
#include <google/protobuf/arena.h>
#include "CompletionToken.h"
#include "BasicPacket.hpp"

namespace Netcode::Network {

	template<typename T>
	class ArenaAllocatorAdapter {
		google::protobuf::Arena * a;
	public:

		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		google::protobuf::Arena * GetArena() const {
			return a;
		}

		~ArenaAllocatorAdapter() = default;
		ArenaAllocatorAdapter() : a{ nullptr } { }
		ArenaAllocatorAdapter(google::protobuf::Arena * a) : a{ a } {}
		ArenaAllocatorAdapter(const ArenaAllocatorAdapter &) = default;
		ArenaAllocatorAdapter(ArenaAllocatorAdapter &&) noexcept = default;
		ArenaAllocatorAdapter & operator=(const ArenaAllocatorAdapter &) = default;
		ArenaAllocatorAdapter & operator=(ArenaAllocatorAdapter &&) noexcept = default;

		template<typename U>
		bool operator==(const ArenaAllocatorAdapter<U> & rhs) const {
			return GetArena() == rhs.GetArena();
		}

		template<typename U>
		ArenaAllocatorAdapter(const ArenaAllocatorAdapter<U> & rhs) : a{ rhs.GetArena() } { }

		template< typename U > struct rebind {
			using other = ArenaAllocatorAdapter<U>;
		};

		T * allocate(std::size_t n) {
			return reinterpret_cast<T *>(google::protobuf::Arena::CreateArray<uint8_t>(a, sizeof(T) * n));
		}

		static void deallocate(T * ptr, std::size_t n) { }
	};

	class NetAllocator : public std::enable_shared_from_this<NetAllocator> {

		// to avoid the pointer storage, and constructor requirement in uniq ptr
		struct RawPtrDeleter {
			void operator()(void * ptr) {
				std::free(ptr);
			}
		};

		// allows the user to retain this block even if a the arena is Reset.
		std::unique_ptr<void, RawPtrDeleter> firstBlock;
		size_t blockSize;
		boost::asio::io_context * ioc;
		google::protobuf::Arena arena;

		static google::protobuf::ArenaOptions GetOptions(void * firstBlock, size_t blockSize) {
			google::protobuf::ArenaOptions ao;
			ao.initial_block = reinterpret_cast<char *>(firstBlock);
			ao.initial_block_size = blockSize;
			ao.max_block_size = blockSize;
			ao.start_block_size = blockSize;
			return ao;
		}

		template<typename T>
		class ArenaOwnerAdapter {
			std::shared_ptr<NetAllocator> alloc;

		public:
			using value_type = T;
			using size_type = std::size_t;
			using difference_type = std::ptrdiff_t;


			[[nodiscard]]
			std::shared_ptr<NetAllocator> GetAllocator() const {
				return alloc;
			}

			T * allocate(size_type n) {
				return alloc->GetAdapter<T>().allocate(n);
			}

			static void deallocate(T * ptr, size_type n) {

			}

			ArenaOwnerAdapter() : alloc{ nullptr } { }

			ArenaOwnerAdapter(std::shared_ptr<NetAllocator> alloc) : alloc{ std::move(alloc) } { }

			template<typename U>
			ArenaOwnerAdapter(const ArenaOwnerAdapter<U> & rhs) : alloc{ rhs.GetAllocator() } {

			}

			template<typename U>
			struct rebind {
				using other = ArenaOwnerAdapter<U>;
			};
		};

	public:
		NetAllocator(boost::asio::io_context * ioc, size_t blockSize) :
			firstBlock{ std::malloc(blockSize) }, blockSize{ blockSize },
			ioc{ ioc },
			arena{ GetOptions(firstBlock.get(), blockSize) } {

		}

		/*
		* Clears the arena and reinitializes it without reallocating the first block.
		* Dangerous method as it'll ignore every live Ref<T>.
		* Main use-case is to reuse allocations when an error happens.
		*/
		void Clear() {
			arena.Reset();
			arena.Init(GetOptions(firstBlock.get(), blockSize));
		}

		google::protobuf::Arena * GetArena() {
			return &arena;
		}

		template<typename T>
		ArenaAllocatorAdapter<T> GetAdapter() {
			return ArenaAllocatorAdapter<T>{ &arena };
		}

		template<typename T>
		T * MakeArray(size_t n) {
			return google::protobuf::Arena::CreateArray<T>(&arena, n);
		}

		template<typename T>
		T * MakeProto() {
			return google::protobuf::Arena::CreateMessage<T>(&arena);
		}

		template<typename T, typename ... U>
		T * Make(U && ... args) {
			return google::protobuf::Arena::Create<T>(&arena, std::forward<U>(args)...);
		}

		
		UdpPacket * MakeUdpPacket(uint32_t requiredPacketSize) {
			UdpPacket * pPacket = Make<UdpPacket>();
			pPacket->SetDataUnsafe(MakeArray<uint8_t>(requiredPacketSize), requiredPacketSize);
			return pPacket;
		}

		template<typename T, typename ... U>
		Ref<T> MakeShared(U&& ... args) {
			return std::allocate_shared<T>(ArenaOwnerAdapter<void>{ shared_from_this() }, std::forward<U>(args)...);
		}

		template<typename T, typename ... U>
		CompletionToken<T> MakeCompletionToken(U && ... args) {
			return std::allocate_shared<CompletionTokenType<T>>(ArenaOwnerAdapter<void>{ shared_from_this() }, ioc, std::forward<U>(args)...);
		}
	};


}
