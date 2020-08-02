#pragma once

#include <string>

namespace Netcode {

	class Uri {
	protected:
		std::wstring fullpath;
	public:

		const std::wstring & GetFullPath();

	};

}
