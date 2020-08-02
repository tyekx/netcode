#pragma once

#include <NetcodeFoundation/Uri.h>

namespace Netcode {

	class AssetUri : public Uri {
	public:
		using Uri::Uri;

		constexpr static std::wstring GetLocalId() {
			return std::wstring_view{ L"Asset/" };
		}

		constexpr static std::wstring GetGlobalId() {
			return std::wstring_view{ L"/Asset/" };
		}
	};

	class ShaderUri final : public AssetUri {
	protected:
		std::wstring_view shaderPath;

		ShaderUri() = default;

		void Construct() {
			auto globalId = GetGlobalId();

			shaderPath = std::wstring_view{ fullPath.c_str() + globalId.size(), fullPath.size() - globalId.size() };
		}

	public:
		ShaderUri(std::wstring fullPath) : AssetUri{ std::move(fullPath) } {
			if(this->fullPath.find(ShaderUri::GetGlobalId(), 0) != 0) {
				this->fullPath.clear();
			} else {
				Construct();
			}
		}

		constexpr static std::wstring_view GetLocalId() {
			return std::wstring_view{ L"Shader/" };
		}

		constexpr static std::wstring_view GetGlobalId() {
			return std::wstring_view{ L"/Asset/Shader/" };
		}

		static ShaderUri Parse(Uri uri) {
			return ShaderUri{ std::move(uri.fullPath) };
		}
	};

	class ShaderVariant final : public AssetUri {
	protected:

	public:

	};

	class ModelUri final : public AssetUri {
	protected:

	public:

	};
}
