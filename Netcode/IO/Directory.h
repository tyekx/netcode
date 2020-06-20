#pragma once

#include <string>
#include <memory>

namespace Netcode::IO {

	class Directory {
		struct detail;
		std::unique_ptr<detail> impl;
	public:

		Directory();
		~Directory();

		static bool Exists(std::wstring_view path);

	};

}
