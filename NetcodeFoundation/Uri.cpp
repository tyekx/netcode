#include "Uri.h"
#include <memory>

namespace Netcode {

	Uri::Uri(std::wstring fullPath) : fullPath{ std::move(fullPath) } { }

	constexpr std::wstring_view Uri::GetLocalId() {
		return std::wstring_view{ L"/" };
	}

	constexpr std::wstring_view Uri::GetGlobalId() {
		return std::wstring_view{ L"/" };
	}

	const std::wstring & Uri::GetFullPath() const {
		return fullPath;
	}
}
