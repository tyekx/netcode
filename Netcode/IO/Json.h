#pragma once

#include <NetcodeFoundation/Json.h>

namespace Netcode::IO {

	void ParseJsonFromFile(JsonDocument & doc, const std::wstring & path);

}
