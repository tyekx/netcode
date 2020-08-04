#include "Shader.h"
#include "../IO/Path.h"
#include <memory>

namespace Netcode::URI {

	Shader::Shader(const wchar_t * relativePath) : AssetBase{}
	{
		std::wstring tmp = relativePath;
		Shader v(std::move(tmp));
		*this = std::move(v);
	}

	Shader::Shader(std::wstring path) : AssetBase{} {
		if(path.empty()) {
			return;
		}

		IO::Path::UnifySlashes(path, L'/');

		if(path.front() == L'/') {
			return;
		}

		ConstructPaths(path, GetGlobalId(), IO::Path::ShaderRoot());
	}

	Shader::Shader(std::wstring uriPath, FullPathToken) : AssetBase{ } {
		IO::Path::UnifySlashes(uriPath, L'/');

		constexpr auto globalId = Shader::GetGlobalId();

		if(uriPath.find(globalId, 0) != 0) {
			return;
		}

		std::wstring_view localPath = { uriPath.c_str() + globalId.size(), uriPath.size() - globalId.size() };

		ConstructPaths(localPath, globalId, IO::Path::ShaderRoot());
	}

	const std::wstring & Shader::GetShaderPath() const
	{
		return assetPath;
	}

	Shader Shader::Parse(UriBase uri) {
		return Shader{ std::move(uri.GetMutableFullPath()), FullPathToken{} };
	}

}
