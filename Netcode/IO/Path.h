#pragma once

#include <string>

namespace Netcode::IO {

	class Path {
		Path() = default;
		~Path() = default;

		static std::wstring workingDirectory;
		static std::wstring shaderRoot;
		static std::wstring mediaRoot;
	public:

		static void SetWorkingDirectiory(const std::wstring & pwd);

		static void SetShaderRoot(const std::wstring & rootDir);

		static void SetMediaRoot(const std::wstring & rootDir);

		static std::wstring_view WorkingDirectory();

		static std::wstring_view ShaderRoot();

		static std::wstring_view MediaRoot();

		static wchar_t GetOppositeSlash(wchar_t slash);

		static wchar_t GetOtherSlash();

		static wchar_t GetSlash();

		static bool HasRelativeSections(const std::wstring & str, wchar_t slash = GetSlash());

		static void UnifySlashes(std::wstring & str, wchar_t desiredSlash);

		static void RemoveRelativeSections(std::wstring & str);

		static void FixFilePath(std::wstring & str);

		static void FixDirectoryPath(std::wstring & dir);

		static bool IsAbsolute(const std::wstring & path);

		static inline bool IsRelative(const std::wstring & path) {
			return !IsAbsolute(path);
		}

	};

}
