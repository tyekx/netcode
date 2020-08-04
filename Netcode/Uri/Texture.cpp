#include "Texture.h"
#include <Netcode/IO/Path.h>

namespace Netcode::URI {

	Texture::Texture(const wchar_t * relativePath) : AssetBase{} {
		std::wstring tmp = relativePath;
		Texture v(std::move(tmp));
		*this = std::move(v);
	}

	Texture::Texture(std::wstring relativePath) : AssetBase{} {
		if(relativePath.empty()) {
			return;
		}

		IO::Path::UnifySlashes(relativePath, L'/');

		if(relativePath.front() == L'/') {
			return;
		}

		ConstructPaths(relativePath, GetGlobalId(), IO::Path::MediaRoot());
	}

	Texture::Texture(std::wstring fullPath, FullPathToken) : AssetBase{} {
		IO::Path::UnifySlashes(fullPath, L'/');

		constexpr auto globalId = Texture::GetGlobalId();

		if(fullPath.find(globalId, 0) != 0) {
			return;
		}

		std::wstring_view localPath = { fullPath.c_str() + globalId.size(), fullPath.size() - globalId.size() };

		ConstructPaths(localPath, globalId, IO::Path::MediaRoot());
	}

	const std::wstring & Texture::GetTexturePath() const {
		return GetAssetPath();
	}

	Texture Texture::Parse(UriBase uri) {
		return Texture(std::move(uri.GetMutableFullPath()), FullPathToken{});
	}

	Texture Texture::AsExtensionOf(const AssetBase & anotherAsset, std::wstring relativeToAssetPath) {
		std::wstring relPath = ExtensionOf(anotherAsset, std::move(relativeToAssetPath), IO::Path::MediaRoot());

		return Texture{ std::move(relPath) };
	}

}
