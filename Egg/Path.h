#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <map>

namespace Egg {

	class Path {
		static std::wstring MediaRoot;
		static std::wstring ShaderRoot;

	public:

		static bool FileExists(const wchar_t * path);

		static void SetMediaRoot(const std::wstring & mediaPath);
		static void SetShaderRoot(const std::wstring & shaderPath);

		static const std::wstring & GetMediaRoot();
		static const std::wstring & GetShaderRoot();

		/*
		* A function to replace any backslashes to forwardslash and make sure the string ends on a slash
		*/
		static std::wstring FixDirectoryPath(const std::wstring & dir);
	};

	class ShaderPath {
		const std::wstring relativePath;
		const std::wstring absolutePath;
	public:
		ShaderPath(const wchar_t * relativeShaderPath);
		ShaderPath(const std::wstring & relativeShaderPath);

		const std::wstring & GetRelativePath() const;
		const std::wstring & GetAbsolutePath() const;
	};

	class MediaPath {
		const std::wstring relativePath;
		const std::wstring absolutePath;
	public:
		MediaPath(const wchar_t * relativeMediaPath);
		MediaPath(const std::wstring & relativeMediaPath);

		const std::wstring & GetRelativePath() const;
		const std::wstring & GetAbsolutePath() const;
	};

}
