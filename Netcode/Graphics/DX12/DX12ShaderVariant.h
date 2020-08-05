#pragma once

#include <Netcode/HandleTypes.h>
#include <Netcode/URI/Shader.h>
#include "DX12Decl.h"
#include <map>
#include <string>
#include <wrl.h>

namespace Netcode::Graphics::DX12 {

	struct ShaderVariantDesc {
		std::map<std::string, std::string> defines;
		std::string entryFunctionName;
		URI::Shader sourceFileUri;
		ShaderType shaderType;

		bool operator==(const ShaderVariantDesc & rhs) const {
			if(entryFunctionName != rhs.entryFunctionName) {
				return false;
			}

			if(sourceFileUri.GetFullPath() != rhs.sourceFileUri.GetFullPath()) {
				return false;
			}

			if(shaderType != rhs.shaderType) {
				return false;
			}

			if(defines.size() != rhs.defines.size()) {
				return false;
			}

			for(auto kv : defines) {
				auto rhsKv = rhs.defines.find(kv.first);

				if(rhsKv == rhs.defines.end()) {
					return false;
				}

				if(kv.second != rhsKv->second) {
					return false;
				}
			}

			return true;
		}
	};

	class ShaderVariant : public ShaderBytecode {
		ShaderVariantDesc desc;
		com_ptr<ID3DBlob> shaderByteCode;

	public:
		ShaderVariant(ShaderVariantDesc desc, com_ptr<ID3DBlob> shaderBytecode);

		virtual const std::wstring & GetFileReference() const override {
			return desc.sourceFileUri.GetShaderPath();
		}

		const ShaderVariantDesc & GetDesc() const;

		virtual uint8_t * GetBufferPointer() override;
		virtual size_t GetBufferSize() override;
	};

}