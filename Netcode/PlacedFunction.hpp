#pragma once

#include <memory>

namespace Netcode {

	template<typename ReturnValue, typename ... Args>
	class FunctorStorageBase {
	public:
		virtual ~FunctorStorageBase() = default;
		virtual ReturnValue operator()(Args&&... args) = 0;
		virtual FunctorStorageBase<ReturnValue, Args...> * MoveInto(void * dst) = 0;
	};

	template<typename Functor, typename ReturnValue, typename ... Args>
	class FunctorStorage : public FunctorStorageBase<ReturnValue, Args...> {
	public:
		Functor f;

		FunctorStorage(Functor && f) : f{ std::move(f) } { }

		virtual ReturnValue operator()(Args&&... args) override {
			return f(std::forward<Args>(args)...);
		}

		virtual FunctorStorageBase<ReturnValue, Args...> * MoveInto(void * dst) override {
			return new (dst) FunctorStorage<Functor, ReturnValue, Args...>{ std::move(f) };
		}
	};

	template<size_t, typename>
	class PlacedFunction;

	template<size_t StorageSize, typename ReturnValue, typename ... Args>
	class PlacedFunction<StorageSize, ReturnValue(Args...)> {
		FunctorStorageBase<ReturnValue, Args...> * functor;
		std::aligned_storage_t<StorageSize> storage;

	public:
		FunctorStorageBase<ReturnValue, Args...> * GetInternalFunctor() {
			return functor;
		}

		PlacedFunction() noexcept : functor{ nullptr }, storage{} {}
		~PlacedFunction() {
			Reset();
		}

		PlacedFunction(PlacedFunction && rhs) noexcept : PlacedFunction{} {
			if(rhs.functor) {
				functor = rhs.functor->MoveInto(std::addressof(storage));
				rhs.Reset();
			}
		}

		template<typename Functor>
		PlacedFunction(Functor f) : PlacedFunction{} {
			static_assert(sizeof(FunctorStorage<Functor, ReturnValue, Args...>) <= StorageSize, "This functor cant fit");
			functor = new (std::addressof(storage)) FunctorStorage<Functor, ReturnValue, Args...>(std::forward<Functor>(f));
		}

		template<size_t RhsSize>
		PlacedFunction & operator=(PlacedFunction<RhsSize, ReturnValue(Args...)> && rhs) noexcept {
			static_assert(RhsSize <= StorageSize, "Narrowing move is invalid");

			Reset();
			if(rhs != nullptr) {
				functor = rhs.GetInternalFunctor()->MoveInto(std::addressof(storage));
				rhs.Reset();
			}
			return *this;
		}

		template<typename Functor>
		PlacedFunction & operator=(Functor f) noexcept {
			Reset();
			static_assert(sizeof(FunctorStorage<Functor, ReturnValue, Args...>) <= StorageSize, "This functor cant fit");
			functor = new (std::addressof(storage)) FunctorStorage<Functor, ReturnValue, Args...>(std::forward<Functor>(f));
			return *this;
		}

		PlacedFunction(const PlacedFunction &) = delete;
		PlacedFunction & operator=(const PlacedFunction &) = delete;

		void Reset() {
			if(functor) {
				functor->~FunctorStorageBase<ReturnValue, Args...>();
				functor = nullptr;
			}
		}

		ReturnValue operator()(Args&& ... args) {
			return functor->operator()(std::forward<Args>(args)...);
		}

		bool operator!=(std::nullptr_t) const noexcept {
			return functor != nullptr;
		}

		bool operator==(std::nullptr_t) const noexcept {
			return functor == nullptr;
		}

	};

}
