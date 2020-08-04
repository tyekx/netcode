#pragma once

#include <NetcodeFoundation/UriBase.h>

namespace Netcode::URI {

	class AssetBase : public UriBase {
	protected:
		std::wstring assetPath;
		std::wstring_view assetDirectory;

		void ConstructPaths(std::wstring_view localPath, std::wstring_view globalId, std::wstring_view rootDir);

		static std::wstring ExtensionOf(const AssetBase & anotherAsset, std::wstring relativeToAssetPath, std::wstring_view rootDir);
	public:
		using UriBase::UriBase;

		std::wstring_view GetAssetDirectory() const;

		const std::wstring & GetAssetPath() const;
	};

}
