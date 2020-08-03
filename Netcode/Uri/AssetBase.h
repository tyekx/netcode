#pragma once

#include <NetcodeFoundation/UriBase.h>

namespace Netcode::URI {

	class AssetBase : public UriBase {
	protected:
		std::wstring assetPath;

		void ConstructPaths(std::wstring_view localPath, std::wstring_view globalId, std::wstring_view rootDir);
	public:
		using UriBase::UriBase;

		const std::wstring & GetAssetPath() const;
	};

}
