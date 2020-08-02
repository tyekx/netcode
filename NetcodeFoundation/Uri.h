#pragma once

#include <string>
#include <string_view>

namespace Netcode {

	class Uri {
	protected:
		static const wchar_t LOCAL_ID[];

		std::wstring fullPath;

		Uri() = default;
	public:
		Uri(std::wstring fullPath);

		constexpr static std::wstring_view GetLocalId();
		constexpr static std::wstring_view GetGlobalId();
		
		bool Empty() const {
			return fullPath.empty();
		}

		const std::wstring & GetFullPath() const;

		template<typename T>
		static T Parse(Uri rhs) {
			return T::Parse(std::move(rhs));
		}
	};

}
