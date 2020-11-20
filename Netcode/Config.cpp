#include "Config.h"
#include <NetcodeFoundation/Formats.h>
#include <NetcodeFoundation/Exceptions.h>
#include "MathExt.h"
#include <boost/property_tree/ptree.hpp>

namespace Netcode {

	// std::any wouldnt compile with clang
	using Ptree = boost::property_tree::basic_ptree<std::wstring, Property>;
	
	static Ptree storage{};

	void Config::LoadReflectedValue(const std::wstring & path, std::wstring_view valueType, const JsonValue & value)
	{
#define CHECK_POD_TYPE(strRep, cppType, accessor) \
if(valueType == strRep) { \
	Set< cppType >(path, static_cast<cppType>(accessor)); \
	return; \
}
		
		CHECK_POD_TYPE(L"u8", uint8_t, value.GetUint());
		CHECK_POD_TYPE(L"u16", uint16_t, value.GetUint());
		CHECK_POD_TYPE(L"u32", uint32_t, value.GetUint());
		CHECK_POD_TYPE(L"u64", uint64_t, value.GetUint64());

		CHECK_POD_TYPE(L"i8", int8_t, value.GetInt());
		CHECK_POD_TYPE(L"i16", int16_t, value.GetInt());
		CHECK_POD_TYPE(L"i32", int32_t, value.GetInt());
		CHECK_POD_TYPE(L"i64", int64_t, value.GetInt64());

		CHECK_POD_TYPE(L"bool", bool, value.GetBool());

		if(valueType == L"string") {
			std::wstring v { std::wstring_view{ value.GetString(), value.GetStringLength() } };

			Set<std::wstring>(path, std::move(v));
			return;
		}

		CHECK_POD_TYPE(L"float", float, value.GetFloat());
		CHECK_POD_TYPE(L"double", double, value.GetDouble());
		CHECK_POD_TYPE(L"Format", DXGI_FORMAT, value.GetUint());

#undef CHECK_POD_TYPE

#define CHECK_NC_MATH_TYPE(strRep, cppType) \
if(valueType == strRep) { \
	cppType tmp; \
	JsonValueConverter<>::ConvertFromJson( value, tmp ); \
	Set< cppType >(path, tmp); \
	return; \
}

	
		CHECK_NC_MATH_TYPE(L"Float2", Float2);
		CHECK_NC_MATH_TYPE(L"Float3", Float3);
		CHECK_NC_MATH_TYPE(L"Float4", Float4);

		CHECK_NC_MATH_TYPE(L"Int2", Int2);
		CHECK_NC_MATH_TYPE(L"Int3", Int3);
		CHECK_NC_MATH_TYPE(L"Int4", Int4);

		CHECK_NC_MATH_TYPE(L"UInt2", UInt2);
		CHECK_NC_MATH_TYPE(L"UInt3", UInt3);
		CHECK_NC_MATH_TYPE(L"UInt4", UInt4);
		
#undef CHECK_NC_MATH_TYPE
	}

	void Config::LoadMembersRecursive(const std::wstring & prefix, const JsonValue & value)
	{
		for(const auto & i : value.GetObject()) {
			std::wstring nameStr = i.name.GetString();
			std::wstring path = (!prefix.empty() ? prefix + L"." : L"") + nameStr;

			size_t indexOfColon = nameStr.find(L":");

			if(indexOfColon != std::string::npos) {
				std::wstring_view type{ nameStr.c_str() + indexOfColon + 1, nameStr.size() - indexOfColon - 1 };

				LoadReflectedValue(path, type, i.value);
			} else {
				UndefinedBehaviourAssertion(i.value.IsObject());

				LoadMembersRecursive(path, i.value);
			}
		}
	}

	void Config::LoadJson(const JsonDocument & document)
	{
		LoadMembersRecursive(L"", document);
	}

	Property & Config::GetProperty(const std::wstring & key) {
		try {
			return storage.get_child(key).data();
		} catch(boost::property_tree::ptree_error & err) {
			throw OutOfRangeException{ "Config property was not found" };
		}
	}

	const Property & Config::GetConstProperty(const std::wstring & key) {
		try {
			return storage.get_child(key).data();
		} catch(boost::property_tree::ptree_error& err) {
			throw OutOfRangeException{ "Config property was not found" };
		}
	}

	const Property & Config::GetConstOptionalProperty(const std::wstring & key) noexcept {
		static Property nullProperty{};

		boost::optional<Ptree&> it = storage.get_child_optional(key);

		if(!it) {
			return nullProperty;
		}
		
		return it->data();
	}

	void Config::SetProperty(const std::wstring & key, Property prop) {
		boost::optional<Ptree &> it = storage.get_child_optional(key);

		if(!it) {
			storage.add(key, std::move(prop));
		} else {
			it->data() = prop;
		}
	}

}
