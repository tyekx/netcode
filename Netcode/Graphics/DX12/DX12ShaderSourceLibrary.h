#pragma once

#include <string>
#include <map>

namespace Netcode::Graphics::DX12 {

	class ShaderSourceLibrary {
		std::map<std::wstring, std::string> lib;
	public:

		const std::string & GetSource(const std::wstring & key);

	};

}
