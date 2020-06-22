#pragma once

#include <string>
#include <any>
#include <boost/property_tree/ptree.hpp>
#include <rapidjson/document.h>

namespace Netcode {
	class Property {
	private:
		std::any storage;
	public:
		Property() : storage{ } { }

		template<typename T>
		Property(const T & rhs) : storage{ rhs } {

		}

		template<typename T>
		Property & operator=(const T & rhs) {
			if(storage.has_value()) {
				Set<T>(rhs);
			} else {
				storage = rhs;
			}
			return *this;
		}

		template<typename T>
		inline const T & Get() const {
			return *(std::any_cast<T>(&storage));
		}

		template<typename T>
		inline void Set(const T & rhs) {
			*(std::any_cast<T>(&storage)) = rhs;
		}
	};

	using Ptree = boost::property_tree::basic_ptree<std::string, Property>;

	class Config {
	public:
		static Ptree storage;

		static void LoadReflectedValue(const std::string & path, std::string_view valueType, const rapidjson::Value & value);

		static void LoadMembersRecursive(const std::string & prefix, const rapidjson::Value & value);

		static void LoadJson(const rapidjson::Document & document);

		template<typename T>
		static inline const T & Get(const boost::property_tree::path & key) {
			return storage.get<Property>(key).Get<T>();
		}

		template<typename T>
		static inline void Set(const boost::property_tree::path & key, const T & value) {
			storage.put<Property>(key, Property{ value });
		}
	};

}
