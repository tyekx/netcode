#include "AssetBase.h"
#include <Netcode/IO/Path.h>

namespace Netcode::URI {

	void AssetBase::ConstructPaths(std::wstring_view localPath, std::wstring_view globalId, std::wstring_view rootDir)
	{
		fullPath.reserve(globalId.size() + localPath.size());
		fullPath.append(globalId);
		fullPath.append(localPath);

		assetPath.reserve(rootDir.size() + localPath.size());
		assetPath.append(rootDir);
		assetPath.append(localPath);

		IO::Path::UnifySlashes(assetPath, IO::Path::GetSlash());
	}

	const std::wstring & AssetBase::GetAssetPath() const {
		return assetPath;
	}

}
