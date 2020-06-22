#pragma once

#include <rapidjson/document.h>

namespace Netcode::IO {

	void ParseJson(rapidjson::Document & doc, std::wstring_view path);

}
