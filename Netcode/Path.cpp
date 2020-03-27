#include "Path.h"
#include "Common.h"
#include <algorithm>

#if defined(EGG_OS_WINDOWS)
#include <Windows.h>
#endif

namespace Netcode {
	std::wstring Path::MediaRoot{};
	std::wstring Path::ShaderRoot{};


	bool Path::FileExists(const wchar_t * path) {
		DWORD attribs = GetFileAttributesW(path);

		return (attribs != INVALID_FILE_ATTRIBUTES) &&
			!(attribs & FILE_ATTRIBUTE_DIRECTORY);
	}

	void Path::SetMediaRoot(const std::wstring & mediaPath) {
		ASSERT(Path::MediaRoot.empty(), "Media Root is already set, its value is not meant to be changed after being initialized");

		Path::MediaRoot = Path::FixDirectoryPath(mediaPath);
	}

	void Path::SetShaderRoot(const std::wstring & shaderPath) {
		ASSERT(Path::ShaderRoot.empty(), "Shader Root is already set, its value is not meant to be changed after being initialized");

		Path::ShaderRoot = Path::FixDirectoryPath(shaderPath);
	}

	const std::wstring & Path::GetMediaRoot() {
		return MediaRoot;
	}
	const std::wstring & Path::GetShaderRoot() {
		return ShaderRoot;
	}

	std::wstring Path::FixDirectoryPath(const std::wstring & dir) {
		if(dir.empty()) {
			return std::wstring{};
		}
		std::wstring cpy = dir;
		std::replace(cpy.begin(), cpy.end(), L'/', L'\\');

		if(cpy[cpy.size() - 1] != L'\\') {
			cpy += '\\';
		}

		return cpy;
	}

	ShaderPath::ShaderPath(const wchar_t * relativeShaderPath) : ShaderPath(std::wstring{ relativeShaderPath }) {}

	ShaderPath::ShaderPath(const std::wstring & relativeShaderPath) :
		relativePath{ relativeShaderPath }, absolutePath{ Path::GetShaderRoot() + relativeShaderPath } {

	}

	const std::wstring & ShaderPath::GetRelativePath() const {
		return relativePath;
	}

	const std::wstring & ShaderPath::GetAbsolutePath() const {
		return absolutePath;
	}

	MediaPath::MediaPath(const wchar_t * relativeMediaPath) : MediaPath(std::wstring{ relativeMediaPath }) { }

	MediaPath::MediaPath(const std::wstring & relativeMediaPath) :
		relativePath{ relativeMediaPath }, absolutePath{ Path::GetMediaRoot() + relativeMediaPath } {

	}

	const std::wstring & MediaPath::GetRelativePath() const {
		return relativePath;
	}

	const std::wstring & MediaPath::GetAbsolutePath() const {
		return absolutePath;
	}
}
