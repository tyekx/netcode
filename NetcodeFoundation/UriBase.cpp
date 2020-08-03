#include "UriBase.h"
#include <memory>

namespace Netcode {

	UriBase::UriBase(std::wstring fullPath) : fullPath{ std::move(fullPath) } { }

	UriBase::UriBase(const wchar_t * cStr) : fullPath{ cStr }
	{
	}

	std::wstring & UriBase::GetMutableFullPath()
	{
		return fullPath;
	}

	const std::wstring & UriBase::GetFullPath() const {
		return fullPath;
	}

}
