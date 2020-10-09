#pragma once



#include <cstdint>
#include <functional>
#include <map>

namespace Netcode {

	using EventToken = int64_t;

	class EventBase {
	public:
		static EventToken NextToken() {
			static EventToken id = 1;
			return id++;
		}
	};

	template<typename StdAlloc, typename ... T>
	class ManagedEvent : public EventBase {
	public:

		using FunctionType = typename std::function<void(T...)>;
		using PairType = typename std::pair<EventToken, FunctionType>;
		using ReboundAllocatorType = typename StdAlloc::template rebind<PairType>::other;
		using ContainerType = std::list<PairType, ReboundAllocatorType>;
	private:
		ContainerType handlers;

	public:
		ManagedEvent(const StdAlloc & alloc) : handlers{ alloc } {

		}

		void Clear() {
			handlers.clear();
		}

		bool Empty() const {
			return handlers.empty();
		}

		EventToken Subscribe(std::function<void(T...)> callback) {
			EventToken t = EventBase::NextToken();
			handlers.emplace_back(t, std::move(callback));
			return t;
		}

		void Erase(EventToken token) {
			for(typename ContainerType::iterator it = handlers.begin(); it != handlers.end(); ++it) {
				if(it->first == token) {
					handlers.erase(it);
					return;
				}
			}
		}

		void Invoke(T ... args) {
			if(handlers.empty()) {
				return;
			}
			
			for(typename ContainerType::iterator it = handlers.begin(); it != handlers.end(); ++it) {
				(it->second)(std::forward<T>(args)...);
			}
		}
	};

	template<typename ... T>
	class Event : public EventBase {
	public:
		using FunctionType = typename std::function<void(T...)>;
		using ContainerType = std::list<std::pair<EventToken, FunctionType>>;
	private:
		ContainerType handlers;

	public:
		EventToken Subscribe(std::function<void(T...)> callback) {
			EventToken t = EventBase::NextToken();
			handlers.emplace_back(t, std::move(callback));
			return t;
		}

		void Erase(EventToken token) {
			for(typename ContainerType::iterator it = handlers.begin(); it != handlers.end(); ++it) {
				if(it->first == token) {
					handlers.erase(token);
				}
			}
		}

		void Invoke(T ... args) {
			for(typename ContainerType::iterator it = handlers.begin(); it != handlers.end(); ++it) {
				(it->second)(std::forward<T>(args)...);
			}
		}
	};

}

