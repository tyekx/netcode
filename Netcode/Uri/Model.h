#pragma once

#include "AssetBase.h"

namespace Netcode::URI {

	class Model final : public AssetBase {
	public:
		Model() = default;
		Model(const wchar_t * relativePath);
		Model(std::wstring relativePath);
		Model(std::wstring fullPath, FullPathToken);

		const std::wstring & GetModelPath() const;

		static Model Parse(UriBase uri);

		static Model AsExtensionOf(const AssetBase & anotherAsset, std::wstring relativeToAssetPath);

		constexpr static std::wstring_view GetGlobalId() {
			return std::wstring_view{ L"/Asset/Model/" };
		}
	};


}
