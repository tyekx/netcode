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

		static bool Create(const std::wstring & path);

		static bool Exists(const std::wstring & path);

	};

}
