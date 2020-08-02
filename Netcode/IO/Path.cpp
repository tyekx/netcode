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

	wchar_t Path::GetOtherSlash() {
#if defined(NETCODE_OS_WINDOWS)
		return L'/';
#else
		return L'\\';
#endif
	}

	wchar_t Path::GetSlash() {
#if defined(NETCODE_OS_WINDOWS)
		return L'\\';
#else
		return L'/';
#endif
	}

	void Path::FixFilePath(std::wstring & str) {
		if(str.empty()) {
			return;
		}

		wchar_t slash = GetSlash();
		wchar_t otherSlash = GetOtherSlash();

		std::replace(str.begin(), str.end(), otherSlash, slash);
	}

	void Path::FixDirectoryPath(std::wstring & dir) {
		if(dir.empty()) {
			return;
		}

		wchar_t slash = GetSlash();
		wchar_t otherSlash = GetOtherSlash();

		std::replace(dir.begin(), dir.end(), otherSlash, slash);

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
