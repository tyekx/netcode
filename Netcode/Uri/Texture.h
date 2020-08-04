#pragma once

#include "AssetBase.h"

namespace Netcode::URI {

	class Texture : public AssetBase {
	public:
		Texture() = default;
		Texture(const wchar_t * relativePath);
		Texture(std::wstring relativePath);
		Texture(std::wstring fullPath, FullPathToken);

		const std::wstring & GetTexturePath() const;

		static Texture Parse(UriBase uri);

		static Texture AsExtensionOf(const AssetBase & anotherAsset, std::wstring relativeToAssetPath);
		
		constexpr static std::wstring_view GetGlobalId() {
			return std::wstring_view{ L"/Asset/Texture/" };
		}
	};

}
