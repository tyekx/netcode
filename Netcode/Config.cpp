#include "Config.h"
#include <NetcodeFoundation/Formats.h>
#include <NetcodeFoundation/Exceptions.h>
#include "MathExt.h"

namespace Netcode {

	Ptree Config::storage{};

	void Config::LoadReflectedValue(const std::string & path, std::string_view valueType, const rapidjson::Value & value)
	{
#define CHECK_POD_TYPE(strRep, cppType, accessor) \
if(valueType == strRep) { \
	Set< cppType >(path, static_cast<cppType>(accessor)); \
	return; \
}
		
		CHECK_POD_TYPE("u8", uint8_t, value.GetUint());
		CHECK_POD_TYPE("u16", uint16_t, value.GetUint());
		CHECK_POD_TYPE("u32", uint32_t, value.GetUint());
		CHECK_POD_TYPE("u64", uint64_t, value.GetUint64());

		CHECK_POD_TYPE("i8", int8_t, value.GetInt());
		CHECK_POD_TYPE("i16", int16_t, value.GetInt());
		CHECK_POD_TYPE("i32", int32_t, value.GetInt());
		CHECK_POD_TYPE("i64", int64_t, value.GetInt64());

		CHECK_POD_TYPE("bool", bool, value.GetBool());

		CHECK_POD_TYPE("float", float, value.GetFloat());
		CHECK_POD_TYPE("double", double, value.GetDouble());
		CHECK_POD_TYPE("Format", DXGI_FORMAT, value.GetUint());

#undef CHECK_POD_TYPE

#define CHECK_NC_MATH_TYPE(strRep, cppType, loaderFunc) \
if(valueType == strRep) { \
	Set< cppType >(path, loaderFunc); \
	return; \
}

		CHECK_NC_MATH_TYPE("Float4", Float4, LoadFloat4(value));
		CHECK_NC_MATH_TYPE("Int2", Int2, LoadInt2(value));
		CHECK_NC_MATH_TYPE("UInt2", UInt2, LoadUInt2(value));
		
#undef CHECK_NC_MATH_TYPE
	}

	void Config::LoadMembersRecursive(const std::string & prefix, const rapidjson::Value & value)
	{
		for(const auto & i : value.GetObject()) {
			std::string nameStr = i.name.GetString();
			std::string path = (!prefix.empty() ? prefix + "." : "") + nameStr;

			size_t indexOfColon = nameStr.find(":");

			if(indexOfColon != std::string::npos) {
				std::string_view type{ nameStr.c_str() + indexOfColon + 1, nameStr.size() - indexOfColon - 1 };

				LoadReflectedValue(path, type, i.value);
			} else {
				UndefinedBehaviourAssertion(i.value.IsObject());

				LoadMembersRecursive(path, i.value);
			}
		}
	}

	void Config::LoadJson(const rapidjson::Document & document)
	{
		storage.clear();

		LoadMembersRecursive("", document);
	}

}
