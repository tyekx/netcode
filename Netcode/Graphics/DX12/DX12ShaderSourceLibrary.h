#pragma once

#include <string>
#include <map>

#include "../../Path.h"
#include "../../Utility.h"

namespace Egg::Graphics::DX12 {

	class ShaderSourceLibrary {
		std::map<std::wstring, std::string> lib;
	public:

		const std::string & GetSource(const std::wstring & key) {

			decltype(lib)::const_iterator it = lib.find(key);

			if(it == lib.end()) {
				ShaderPath path{ key };

				std::string dst;
				
				ASSERT(Egg::Utility::SlurpFile(dst, path.GetAbsolutePath()), "File not found");

				lib[key] = std::move(dst);

				it = lib.find(key);
			}

			return it->second;
		}

	};

}
