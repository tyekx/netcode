#include "Shader.h"
#include "../IO/Path.h"
#include <memory>

namespace Netcode::URI {

	void Shader::ParseShaderPath(std::wstring_view path)
	{
		constexpr auto globalId = Shader::GetGlobalId();
		auto shaderRoot = IO::Path::ShaderRoot();

		fullPath.reserve(globalId.size() + path.size());
		fullPath.append(globalId);
		fullPath.append(path);

		shaderPath.reserve(shaderRoot.size() + path.size());
		shaderPath.append(shaderRoot);
		shaderPath.append(path);

		IO::Path::UnifySlashes(shaderPath, IO::Path::GetSlash());
	}

	Shader::Shader(const wchar_t * relativePath) : UriBase{}
	{
		std::wstring tmp = relativePath;
		Shader v(std::move(tmp));
		*this = std::move(v);
	}

	Shader::Shader(std::wstring path) : UriBase {} {

		if(path.empty()) {
			return;
		}

		IO::Path::UnifySlashes(path, L'/');

		if(path.front() == L'/') {
			return;
		}

		ParseShaderPath(path);
	}

	Shader::Shader(std::wstring uriPath, FullPathToken) : UriBase { } {
		IO::Path::UnifySlashes(uriPath, L'/');

		constexpr auto globalId = Shader::GetGlobalId();

		if(uriPath.find(globalId, 0) != 0) {
			return;
		}

		std::wstring_view sPath = { uriPath.c_str() + globalId.size(), uriPath.size() - globalId.size() };

		ParseShaderPath(sPath);
	}

	const std::wstring & Shader::GetShaderPath() const
	{
		return shaderPath;
	}

	Shader Shader::Parse(UriBase uri) {
		return Shader{ std::move(uri.GetMutableFullPath()), FullPathToken{} };
	}

}
