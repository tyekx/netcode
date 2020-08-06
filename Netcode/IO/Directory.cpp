#include <NetcodeFoundation/Platform.h>

#include "Directory.h"

#if defined(NETCODE_OS_WINDOWS)
#include <Windows.h>

#if defined(NETCODE_EDITOR_VARIANT)
#include <fileapifromapp.h>
#endif
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
#if defined(NETCODE_OS_WINDOWS)
#if defined(NETCODE_EDITOR_VARIANT)
		WIN32_FILE_ATTRIBUTE_DATA attrData;

		BOOL success = GetFileAttributesExFromAppW(path.c_str(), GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &attrData);

		return success && (attrData.dwFileAttributes != INVALID_FILE_ATTRIBUTES) && (attrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
		DWORD attribs = GetFileAttributesW(path.data());

		return (attribs != INVALID_FILE_ATTRIBUTES) &&
			(attribs & FILE_ATTRIBUTE_DIRECTORY);
#endif
#else

#endif
	}

}

