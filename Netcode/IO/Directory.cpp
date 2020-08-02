#include <NetcodeFoundation/Platform.h>

#include "Directory.h"

#if defined(NETCODE_OS_WINDOWS)
#include <Windows.h>
#endif

namespace Netcode::IO {

	struct Directory::detail {

	};

	Directory::Directory() : impl{ new detail{} }
	{
	}

	Directory::~Directory()
	{
		impl.reset();
	}

	bool Directory::Exists(const std::wstring & path) {
		DWORD attribs = GetFileAttributesW(path.data());

		return (attribs != INVALID_FILE_ATTRIBUTES) &&
			   (attribs & FILE_ATTRIBUTE_DIRECTORY);
	}

}

