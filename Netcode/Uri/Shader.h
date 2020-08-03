#pragma once

#include <NetcodeFoundation/UriBase.h>

namespace Netcode::URI {

	class Shader final : public UriBase {
		std::wstring shaderPath;

		void ParseShaderPath(std::wstring_view path);
	public:

		Shader() = default;
		Shader(const wchar_t * relativePath);
		Shader(std::wstring relativePath);
		Shader(std::wstring fullPath, FullPathToken);

		const std::wstring & GetShaderPath() const;

		static Shader Parse(UriBase uri);

		constexpr static std::wstring_view GetLocalId() {
			return std::wstring_view{ L"Shader/" };
		}

		constexpr static std::wstring_view GetGlobalId() {
			return std::wstring_view{ L"/Asset/Shader/" };
		}
	};

}
