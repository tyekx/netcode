#include <NetcodeFoundation/Platform.h>
#include <NetcodeFoundation/Exceptions.h>

#include "Path.h"
#include "Directory.h"

#include <algorithm>

namespace Netcode::IO {

	std::wstring Path::workingDirectory{};
	std::wstring Path::shaderRoot{};
	std::wstring Path::mediaRoot{};

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

			std::basic_string_view<const wchar_t> section{ str.c_str() + firstIndex, lastIndex - firstIndex };

			if(section == L"." || section == L"..") {
				return true;
			}

			firstIndex = lastIndex + 1;
		} while(firstIndex < str.size());

		return false;
	}

	void Path::UnifySlashes(std::wstring & str, wchar_t desiredSlash) {
		wchar_t slash = desiredSlash;
		wchar_t oppositeSlash = GetOppositeSlash(slash);

		std::replace(str.begin(), str.end(), oppositeSlash, slash);
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

	void Path::FixFilePath(std::wstring & str) {
		if(str.empty()) {
			return;
		}

		const wchar_t slash = GetSlash();

		UnifySlashes(str, slash);

		UndefinedBehaviourAssertion(str.back() != slash);
	}

	void Path::FixDirectoryPath(std::wstring & dir) {
		if(dir.empty()) {
			return;
		}

		wchar_t slash = GetSlash();

		UnifySlashes(dir, slash);

		if(!dir.empty() && dir.back() != slash) {
			dir.push_back(slash);
		}
	}

	bool Path::IsAbsolute(const std::wstring & path)
	{
#if defined (NETCODE_OS_WINDOWS)
		return path.find_first_of(L':') != std::wstring::npos;
#else
		return !path.empty() && path.at(0) == GetSlash();
#endif
	}
}
