#pragma once

#include <string>
#include <boost/any.hpp>
#include <boost/property_tree/ptree.hpp>
#include <NetcodeFoundation/Json.h>

namespace Netcode {

	// std::any wouldnt compile with clang
	using Ptree = boost::property_tree::basic_ptree<std::wstring, boost::any>;

	class Config {
	public:
		static Ptree storage;

		static void LoadReflectedValue(const std::wstring & path, std::wstring_view valueType, const JsonValue & value);

		static void LoadMembersRecursive(const std::wstring & prefix, const JsonValue & value);

		static void LoadJson(const JsonDocument & document);

		template<typename T>
		static inline const T & Get(const std::wstring & key) {
			return boost::any_cast<const T &>(storage.get_child(key).data());
		}

		template<typename T>
		static inline void Set(const std::wstring & key, const T & value) {
			storage.put(key, boost::any{ value });
		}
	};

}
