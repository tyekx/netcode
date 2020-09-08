#pragma once

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "Math.h"
#include "Exceptions.h"
#include "ArrayView.hpp"

namespace Netcode {

	using JsonDocument = rapidjson::GenericDocument<rapidjson::UTF16LE<wchar_t>>;
	using JsonValue = JsonDocument::ValueType;

	inline const JsonValue& GetMember(const JsonValue & json, const JsonDocument::Ch * str) {
		if(!json.IsObject()) {
			throw OutOfRangeException{ "JSON is not an object" };
		}

		if(const auto it = json.FindMember(str); it != json.MemberEnd()) {
			return it->value;
		}

		throw OutOfRangeException{ "JSON member was not found" };
	}
	
	template<typename ChType>
	class JsonStringBuffer final {
	public:
		using Ch = ChType;

	private:
		std::basic_string<Ch> & strRef;

	public:
		JsonStringBuffer(std::basic_string<Ch> & stringRef) : strRef{ stringRef } {
			strRef.reserve(4096);
		}

		void Flush() { }

		void Put(Ch c) {
			strRef.push_back(c);
		}
	};

	template<typename DocType>
	class JsonSerializer {
	public:
		static void Store(std::basic_string<typename DocType::Ch> & dest, const DocType & jsonToStore) {
			JsonStringBuffer<typename DocType::Ch> buffer{ dest };
			rapidjson::Writer<typename DocType::Ch, typename DocType::EncodingType, typename DocType::EncodingType> writer{ buffer };
			jsonToStore.Accept(writer);
		}

		template<typename SourceEncoding = rapidjson::UTF16LE<wchar_t>>
		static void Load(DocType & jsonToLoad, ArrayView<uint8_t> buffer) {
			rapidjson::MemoryStream memoryStream{ reinterpret_cast<const char *>(buffer.Data()), buffer.Size() };
			rapidjson::EncodedInputStream<SourceEncoding, rapidjson::MemoryStream> encodedInputStream{ memoryStream };
			jsonToLoad.template ParseStream<rapidjson::kParseDefaultFlags, SourceEncoding>(encodedInputStream);
		}
	};

	template<typename DocType = JsonDocument>
	class JsonValueConverter {

		constexpr static void ThrowIf(bool exceptionThrownIfTrue, const char * msg) {
			if(exceptionThrownIfTrue) {
				throw UndefinedBehaviourException{ msg };
			}
		}
		
	public:
		using V = typename DocType::ValueType;
		using A = typename DocType::AllocatorType;
		using C = typename DocType::Ch;
		using StdString = std::basic_string < C >;

		constexpr static void ConvertToJson(const C * value, V & json, A & allocator) {
			json.SetString(value, allocator);
		}
		
		constexpr static void ConvertToJson(const StdString & value, V & json, A & allocator) {
			json.SetString(value, allocator);
		}
		
		constexpr static void ConvertToJson(int32_t value, V & json, A & allocator) {
			(void)allocator;
			json.SetInt(value);
		}

		constexpr static void ConvertToJson(uint32_t value, V & json, A & allocator) {
			(void)allocator;
			json.SetUint(value);
		}
		
		constexpr static void ConvertToJson(float value, V & json, A & allocator) {
			(void)allocator;
			json.SetFloat(value);
		}

		constexpr static void ConvertToJson(double value, V & json, A & allocator) {
			(void)allocator;
			json.SetDouble(value);
		}
		
		constexpr static void ConvertToJson(Float2 f, V & json, A & allocator) {
			json.SetArray();
			json.PushBack(f.x, allocator);
			json.PushBack(f.y, allocator);
		}

		constexpr static void ConvertToJson(Float3 f, V & json, A & allocator) {
			json.SetArray();
			json.PushBack(f.x, allocator);
			json.PushBack(f.y, allocator);
			json.PushBack(f.z, allocator);
		}

		constexpr static void ConvertToJson(Float4 f, V & json, A & allocator) {
			json.SetArray();
			json.PushBack(f.x, allocator);
			json.PushBack(f.y, allocator);
			json.PushBack(f.z, allocator);
			json.PushBack(f.w, allocator);
		}

		constexpr static void ConvertToJson(UInt2 value, V & json, A & allocator) {
			json.SetArray();
			json.PushBack(value.x, allocator);
			json.PushBack(value.y, allocator);
		}

		constexpr static void ConvertToJson(UInt3 value, V & json, A & allocator) {
			json.SetArray();
			json.PushBack(value.x, allocator);
			json.PushBack(value.y, allocator);
			json.PushBack(value.z, allocator);
		}

		constexpr static void ConvertToJson(UInt4 value, V & json, A & allocator) {
			json.SetArray();
			json.PushBack(value.x, allocator);
			json.PushBack(value.y, allocator);
			json.PushBack(value.z, allocator);
			json.PushBack(value.w, allocator);
		}

		constexpr static void ConvertToJson(Int2 value, V & json, A & allocator) {
			json.SetArray();
			json.PushBack(value.x, allocator);
			json.PushBack(value.y, allocator);
		}

		constexpr static void ConvertToJson(Int3 value, V & json, A & allocator) {
			json.SetArray();
			json.PushBack(value.x, allocator);
			json.PushBack(value.y, allocator);
			json.PushBack(value.z, allocator);
		}

		constexpr static void ConvertToJson(Int4 value, V & json, A & allocator) {
			json.SetArray();
			json.PushBack(value.x, allocator);
			json.PushBack(value.y, allocator);
			json.PushBack(value.z, allocator);
			json.PushBack(value.w, allocator);
		}

		constexpr static void ConvertFromJson(const V & json, StdString & value) {
			ThrowIf(!json.IsString(), "Invalid conversion attempt (string)");
			value.assign(json.GetString(), json.GetStringLength());
		}

		constexpr static void ConvertFromJson(const V & json, int32_t & value) {
			ThrowIf(!json.IsInt(), "Invalid conversion attempt (Int)");
			value = json.GetInt();
		}

		constexpr static void ConvertFromJson(const V & json, uint32_t & value) {
			ThrowIf(!json.IsUint(), "Invalid conversion attempt (UInt)");
			value = json.GetUint();
		}
		
		constexpr static void ConvertFromJson(const V & json, float & value) {
			ThrowIf(!json.IsNumber(), "Invalid conversion attempt (float)");
			value = json.GetFloat();
		}
		
		constexpr static void ConvertFromJson(const V & json, double & value) {
			ThrowIf(!json.IsDouble(), "Invalid conversion attempt (double)");
			value = json.GetDouble();
		}
		
		constexpr static void ConvertFromJson(const V & json, Float2 & value) {
			ThrowIf(!json.IsArray(), "Invalid conversion attempt (Float2)");
			typename V::ConstArray jsonArray = json.GetArray();
			
			ThrowIf(jsonArray.Size() != 2, "Invalid size for conversion (Float2)");
			ThrowIf(!jsonArray[0].IsNumber() || !jsonArray[1].IsNumber(),
				"Invalid content for conversion (Float2)");

			value.x = jsonArray[0].GetFloat();
			value.y = jsonArray[1].GetFloat();
		}

		constexpr static void ConvertFromJson(const V & json, Float3 & value) {
			ThrowIf(!json.IsArray(), "Invalid conversion attempt (Float3)");
			typename V::ConstArray jsonArray = json.GetArray();
			
			ThrowIf(jsonArray.Size() != 3, "Invalid size for conversion (Float3)");
			ThrowIf(!jsonArray[0].IsNumber() || !jsonArray[1].IsNumber() || !jsonArray[2].IsNumber(),
				"Invalid content for conversion (Float3)");

			value.x = jsonArray[0].GetFloat();
			value.y = jsonArray[1].GetFloat();
			value.z = jsonArray[2].GetFloat();
		}

		constexpr static void ConvertFromJson(const V & json, Float4 & value) {
			ThrowIf(!json.IsArray(), "Invalid conversion attempt (Float4)");
			
			typename V::ConstArray jsonArray = json.GetArray();
			ThrowIf(jsonArray.Size() != 4, "Invalid size for conversion (Float4)");
			ThrowIf(!jsonArray[0].IsNumber() || !jsonArray[1].IsNumber() ||
				    !jsonArray[2].IsNumber() || !jsonArray[3].IsNumber(),
				"Invalid content for conversion (Float4)");
			
			value.x = jsonArray[0].GetFloat();
			value.y = jsonArray[1].GetFloat();
			value.z = jsonArray[2].GetFloat();
			value.w = jsonArray[3].GetFloat();
		}

		constexpr static void ConvertFromJson(const V & json, Int2 & value) {
			ThrowIf(!json.IsArray(), "Invalid conversion attempt (Int2)");
			typename V::ConstArray jsonArray = json.GetArray();

			ThrowIf(jsonArray.Size() != 2, "Invalid size for conversion (Int2)");
			ThrowIf(!jsonArray[0].IsInt() || !jsonArray[1].IsInt(),
				"Invalid content for conversion (Int2)");

			value.x = jsonArray[0].GetInt();
			value.y = jsonArray[1].GetInt();
		}

		constexpr static void ConvertFromJson(const V & json, Int3 & value) {
			ThrowIf(!json.IsArray(), "Invalid conversion attempt (Int3)");
			typename V::ConstArray jsonArray = json.GetArray();

			ThrowIf(jsonArray.Size() != 3, "Invalid size for conversion (Int3)");
			ThrowIf(!jsonArray[0].IsInt() || !jsonArray[1].IsInt() || !jsonArray[2].IsInt(),
				"Invalid content for conversion (Int3)");
			
			value.x = jsonArray[0].GetInt();
			value.y = jsonArray[1].GetInt();
			value.z = jsonArray[2].GetInt();
		}

		constexpr static void ConvertFromJson(const V & json, Int4 & value) {
			ThrowIf(!json.IsArray(), "Invalid conversion attempt (Int4)");

			typename V::ConstArray jsonArray = json.GetArray();
			ThrowIf(jsonArray.Size() != 4, "Invalid size for conversion (Int4)");
			ThrowIf(!jsonArray[0].IsInt() || !jsonArray[1].IsInt() ||
					!jsonArray[2].IsInt() || !jsonArray[3].IsInt(),
					"Invalid content for conversion (Int4)");

			value.x = jsonArray[0].GetInt();
			value.y = jsonArray[1].GetInt();
			value.z = jsonArray[2].GetInt();
			value.w = jsonArray[3].GetInt();
		}
		
		constexpr static void ConvertFromJson(const V & json, UInt2 & value) {
			ThrowIf(!json.IsArray(), "Invalid conversion attempt (UInt2)");
			typename V::ConstArray jsonArray = json.GetArray();

			ThrowIf(jsonArray.Size() != 2, "Invalid size for conversion (UInt2)");
			ThrowIf(!jsonArray[0].IsUint() || !jsonArray[1].IsUint(),
				"Invalid content for conversion (UInt2)");

			value.x = jsonArray[0].GetUint();
			value.y = jsonArray[1].GetUint();
		}
		
		constexpr static void ConvertFromJson(const V & json, UInt3 & value) {
			ThrowIf(!json.IsArray(), "Invalid conversion attempt (UInt3)");
			typename V::ConstArray jsonArray = json.GetArray();

			ThrowIf(jsonArray.Size() != 3, "Invalid size for conversion (UInt3)");
			ThrowIf(!jsonArray[0].IsUint() || !jsonArray[1].IsUint() || !jsonArray[2].IsUint(),
				"Invalid content for conversion (UInt3)");

			value.x = jsonArray[0].GetUint();
			value.y = jsonArray[1].GetUint();
			value.z = jsonArray[2].GetUint();
		}

		constexpr static void ConvertFromJson(const V & json, UInt4 & value) {
			ThrowIf(!json.IsArray(), "Invalid conversion attempt (UInt4)");

			typename V::ConstArray jsonArray = json.GetArray();
			ThrowIf(jsonArray.Size() != 4, "Invalid size for conversion (UInt4)");
			ThrowIf(!jsonArray[0].IsUint() || !jsonArray[1].IsUint() ||
				!jsonArray[2].IsUint() || !jsonArray[3].IsUint(),
				"Invalid content for conversion (UInt4)");

			value.x = jsonArray[0].GetUint();
			value.y = jsonArray[1].GetUint();
			value.z = jsonArray[2].GetUint();
			value.w = jsonArray[3].GetUint();
		}
	};
	
}
