#pragma once

#include <boost/any.hpp>
#include <exception>
#include <map>
#include <string>

class Config {
	Config() = delete;
	~Config() = delete;
	Config(const Config &) = delete;
	Config(Config &&) = delete;
	void operator=(const Config &) = delete;
	void operator=(Config &&) = delete;

	static std::map<std::string, boost::any> Data;
public:
	
	template<typename T>
	static const T & Get(const std::string & key) {
		if(Data.find(key) != Data.end()) {
			return boost::any_cast<const T &>(Data[key]);
		}
		throw std::invalid_argument("Key not found");
	}

	template<typename T>
	static void Set(const std::string & key, const T & value) {
		Data[key] = value;
	}
};

