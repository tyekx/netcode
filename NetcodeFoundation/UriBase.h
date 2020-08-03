#pragma once

#include <string>
#include <string_view>

namespace Netcode {

	struct FullPathToken {

	};

	class UriBase {
	protected:
		std::wstring fullPath;

	public:
		UriBase() = default;
		UriBase(std::wstring fullPath);
		UriBase(const wchar_t * cStr);

		constexpr static std::wstring_view GetLocalId() {
			return std::wstring_view{ L"/" };
		}

		constexpr static std::wstring_view GetGlobalId() {
			return std::wstring_view{ L"/" };
		}
		
		bool Empty() const {
			return fullPath.empty();
		}

		std::wstring & GetMutableFullPath();

		const std::wstring & GetFullPath() const;

		template<typename T>
		static T Parse(UriBase rhs) {
			return T::Parse(std::move(rhs));
		}
	};

}
