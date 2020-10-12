#pragma once

#include <string>

namespace Netcode::IO {

	class Path {
		Path() = default;
		~Path() = default;

		static std::wstring workingDirectory;
		static std::wstring shaderRoot;
		static std::wstring mediaRoot;
		static std::wstring appData;
	public:

		static void SetWorkingDirectiory(const std::wstring & pwd);

		static void SetShaderRoot(const std::wstring & rootDir);

		static void SetMediaRoot(const std::wstring & rootDir);

		static std::wstring_view WorkingDirectory();

		static std::wstring_view ShaderRoot();

		static std::wstring_view MediaRoot();

		static std::wstring_view AppData();

		static wchar_t GetOppositeSlash(wchar_t slash);

		static wchar_t GetOtherSlash();

		static wchar_t GetSlash();

		static bool HasRelativeSections(const std::wstring & str, wchar_t slash = GetSlash());

		static std::wstring CurrentWorkingDirectory();

		static void UnifySlashes(std::wstring & str, wchar_t desiredSlash);

		static bool CheckSlashConsistency(std::wstring_view str, wchar_t expectedSlash = GetSlash());

		static void RemoveRelativeSections(std::wstring & str);

		static void FixFilePath(std::wstring & str, wchar_t desiredSlash = GetSlash());

		static void FixDirectoryPath(std::wstring & dir, wchar_t desiredSlash = GetSlash());

		static bool IsDirectory(std::wstring_view str, wchar_t desiredSlash = GetSlash());

		static std::wstring_view GetParentDirectory(std::wstring_view str, wchar_t desiredSlash = GetSlash());

		static bool IsAbsolute(std::wstring_view str);

		static inline bool IsRelative(std::wstring_view str) {
			return !IsAbsolute(str);
		}

	};

}
