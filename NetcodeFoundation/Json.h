#pragma once

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

namespace Netcode::Json {

	using Document = rapidjson::GenericDocument<rapidjson::UTF16LE<wchar_t>>;
	using Value = rapidjson::GenericValue<rapidjson::UTF16LE<wchar_t>, Document::AllocatorType>;
	using Buffer = rapidjson::GenericStringBuffer<Document::EncodingType>;

}
