#include <NetcodeFoundation/Platform.h>
#include <NetcodeFoundation/Exceptions.h>

#include "Path.h"
#include "Directory.h"

#include <algorithm>

#if defined(NETCODE_OS_WINDOWS)
#include <Windows.h>
#include <ShlObj.h>
#endif

namespace Netcode::IO {

	std::wstring Path::workingDirectory{};
	std::wstring Path::shaderRoot{};
	std::wstring Path::mediaRoot{};
	std::wstring Path::appData{};

	void Path::SetWorkingDirectiory(const std::wstring & pwd) {
		UndefinedBehaviourAssertion(!pwd.empty());
		UndefinedBehaviourAssertion(workingDirectory.empty());

		workingDirectory = pwd;
		Path::FixDirectoryPath(workingDirectory);
	}

	void Path::SetShaderRoot(const std::wstring & rootDir) {
		UndefinedBehaviourAssertion(!rootDir.empty());
		UndefinedBehaviourAssertion(shaderRoot.empty());

		shaderRoot = rootDir;
		Path::FixDirectoryPath(shaderRoot);
	}

	void Path::SetMediaRoot(const std::wstring & rootDir) {
		UndefinedBehaviourAssertion(!rootDir.empty());
		UndefinedBehaviourAssertion(mediaRoot.empty());

		mediaRoot = rootDir;
		Path::FixDirectoryPath(mediaRoot);
	}

	std::wstring_view Path::WorkingDirectory() {
		return workingDirectory;
	}

	std::wstring_view Path::ShaderRoot() {
		return shaderRoot;
	}

	std::wstring_view Path::MediaRoot() {
		return mediaRoot;
	}

	std::wstring_view Path::AppData()
	{
		if(appData.empty()) {
			wchar_t data[MAX_PATH];
			SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, data);
			appData = std::wstring_view{ data, wcslen(data) };
			FixDirectoryPath(appData);
		}

		return appData;
	}

	wchar_t Path::GetOppositeSlash(wchar_t slash)
	{
		if(slash == L'/') {
			return L'\\';
		} else {
			return L'/';
		}
	}

	wchar_t Path::GetOtherSlash() {
		return GetOppositeSlash(GetSlash());
	}

	wchar_t Path::GetSlash() {
#if defined(NETCODE_OS_WINDOWS)
		return L'\\';
#else
		return L'/';
#endif
	}

	bool Path::HasRelativeSections(const std::wstring & str, wchar_t slash)
	{
		size_t firstIndex = 0;
		size_t lastIndex = std::wstring::npos;

		do {
			lastIndex = str.find(slash, firstIndex);

			if(lastIndex == std::wstring::npos) {
				lastIndex = str.size();
			}

			std::wstring_view section{ str.c_str() + firstIndex, lastIndex - firstIndex };

			if(section == L"." || section == L"..") {
				return true;
			}

			firstIndex = lastIndex + 1;
		} while(firstIndex < str.size());

		return false;
	}

	std::wstring Path::CurrentWorkingDirectory() {
#if defined(NETCODE_OS_WINDOWS)
		DWORD len = GetCurrentDirectoryW(0, nullptr);
		std::wstring pwd;
		pwd.resize(len);
		GetCurrentDirectoryW(static_cast<DWORD>(pwd.size()), pwd.data());
		pwd.resize(len - 1);
		return pwd;
#else
		/* not implemented */
#endif
	}

	void Path::UnifySlashes(std::wstring & str, wchar_t desiredSlash) {
		wchar_t slash = desiredSlash;
		wchar_t oppositeSlash = GetOppositeSlash(slash);

		std::replace(str.begin(), str.end(), oppositeSlash, slash);
	}

	bool Path::CheckSlashConsistency(std::wstring_view str, wchar_t expectedSlash) {
		wchar_t oppositeSlash = GetOppositeSlash(expectedSlash);

		return str.find(oppositeSlash) == std::wstring::npos;
	}

	void Path::RemoveRelativeSections(std::wstring & str) {
		const wchar_t slash = GetSlash();

		std::wstring strCopy = str;

		const std::wstring slashStr = { slash };
		const std::wstring slashDotDot = { slash, L'.', L'.' };
		const std::wstring slashDotSlash = { slash, L'.', slash };

		size_t indexOf = std::wstring::npos;

		while((indexOf = strCopy.find(slashDotSlash)) != std::wstring::npos) {
			strCopy.replace(std::begin(strCopy) + indexOf, std::begin(strCopy) + indexOf + slashDotSlash.size(), slashStr);
		}

		while((indexOf = strCopy.find(slashDotDot)) != std::wstring::npos) {

			size_t lastSlash = strCopy.rfind(slash, indexOf - 1);

			if(lastSlash == std::wstring::npos) {
				return;
			}

			strCopy.erase(lastSlash, indexOf - lastSlash + slashDotDot.size());
		}

		str = std::move(strCopy);
	}

	void Path::FixFilePath(std::wstring & str, wchar_t desiredSlash) {
		if(str.empty()) {
			return;
		}

		UnifySlashes(str, desiredSlash);

		UndefinedBehaviourAssertion(str.back() != desiredSlash);
	}

	void Path::FixDirectoryPath(std::wstring & dir, wchar_t desiredSlash) {
		if(dir.empty()) {
			return;
		}

		UnifySlashes(dir, desiredSlash);

		if(!dir.empty() && dir.back() != desiredSlash) {
			dir.push_back(desiredSlash);
		}
	}

	bool Path::IsDirectory(std::wstring_view str, wchar_t slash)
	{
		return !str.empty() && str.back() == slash;
	}

	std::wstring_view Path::GetParentDirectory(std::wstring_view str, wchar_t slash)
	{
		if(str.empty()) {
			return std::wstring_view{};
		}

		if(str.size() == 1 && str[0] == slash) {
			return str;
		}

		if(IsDirectory(str, slash)) {
			str = std::wstring_view{ str.data(), str.size() - 1 };
		}

		size_t idx = str.find_last_of(slash);

		if(IsRelative(str)) {
			if(idx == std::wstring::npos) {
				return WorkingDirectory();
			} else {
				return std::wstring_view{ str.data(), idx + 1 };
			}
		} else {
			if(idx == std::wstring::npos) {
				return str;
			} else {
				return std::wstring_view{ str.data(), idx + 1 };
			}
		}
	}

	bool Path::IsAbsolute(std::wstring_view str)
	{
#if defined (NETCODE_OS_WINDOWS)
		return str.find_first_of(L':') != std::wstring::npos;
#else
		return !str.empty() && str[0] == GetSlash();
#endif
	}
}
