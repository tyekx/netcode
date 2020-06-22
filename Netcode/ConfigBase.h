#pragma once

#include <string>
#include <any>
#include <boost/property_tree/ptree.hpp>

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

	class ConfigBase {
	protected:
		Ptree * node;

	public:
		ConfigBase(Ptree & parentNode, const char * nodeName) : node{ &parentNode.add_child(nodeName, Ptree{}) } { }
	};

}
