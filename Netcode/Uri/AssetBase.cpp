#include "AssetBase.h"
#include <Netcode/IO/Path.h>

namespace Netcode::URI {
	
	AssetBase::AssetBase(const AssetBase & rhs) : UriBase{ rhs }, assetPath{ rhs.assetPath }, assetDirectory{  } {
		if(!assetPath.empty()) {
			assetDirectory = std::wstring_view{ assetPath.c_str(), rhs.assetDirectory.size() };
		}
	}

	AssetBase & AssetBase::operator=(const AssetBase & rhs)
	{
		UriBase::operator=(rhs);
		if(rhs.Empty()) {
			assetPath = rhs.assetPath;
			assetDirectory = std::wstring_view{};
		} else {
			assetPath = rhs.assetPath;
			assetDirectory = std::wstring_view{ assetPath.c_str(), rhs.assetDirectory.size() };
		}
		return *this;
	}

	void AssetBase::ConstructPaths(std::wstring_view localPath, std::wstring_view globalId, std::wstring_view rootDir)
	{
		fullPath.reserve(globalId.size() + localPath.size());
		fullPath.append(globalId);
		fullPath.append(localPath);

		assetPath.reserve(rootDir.size() + localPath.size());
		assetPath.append(rootDir);
		assetPath.append(localPath);

		IO::Path::UnifySlashes(assetPath, IO::Path::GetSlash());

		assetDirectory = IO::Path::GetParentDirectory(assetPath);
	}

	std::wstring AssetBase::ExtensionOf(const AssetBase & anotherAsset, std::wstring relativeToAssetPath, std::wstring_view rootDir)
	{
		std::wstring_view assetDir = anotherAsset.GetAssetDirectory();

		if(assetDir.find(rootDir) != 0) {
			return relativeToAssetPath;
		}

		std::wstring_view mediaRelativePath{ assetDir.data() + rootDir.size(), assetDir.size() - rootDir.size() };

		std::wstring prefixedRelativePath;
		prefixedRelativePath.reserve(mediaRelativePath.size() + relativeToAssetPath.size());
		prefixedRelativePath.append(mediaRelativePath);
		prefixedRelativePath.append(std::move(relativeToAssetPath));
		IO::Path::UnifySlashes(prefixedRelativePath, L'/');

		return prefixedRelativePath;
	}

	std::wstring_view AssetBase::GetAssetDirectory() const {
		return assetDirectory;
	}

	const std::wstring & AssetBase::GetAssetPath() const {
		return assetPath;
	}

}
