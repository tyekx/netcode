#pragma once

#include <cstdint>
#include <functional>
#include <map>

namespace Netcode {

	using EventToken = int64_t;

	template<typename ... T>
	class Event {
		EventToken tokenGenerator;

		using func_t = typename std::function<void(T...)>;

		std::map<EventToken, func_t> callbacks;


	public:
		EventToken Subscribe(std::function<void(T...)> callback) {
			EventToken t = tokenGenerator++;
			callbacks[t] = callback;
			return t;
		}

		void Erase(EventToken token) {
			callbacks.erase(token);
		}

		void Invoke(T ... args) {
			for(auto it = callbacks.begin(); it != callbacks.end(); ++it) {
				(it->second)(std::forward<T>(args)...);
			}
		}
	};
}

