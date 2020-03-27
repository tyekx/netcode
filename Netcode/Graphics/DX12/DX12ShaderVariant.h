#pragma once

#include "../../HandleTypes.h"

#include <vector>
#include <iostream>
#include <string>
#include <map>

#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	struct ShaderVariantDesc {
		std::map<std::string, std::string> defines;
		std::string entryFunctionName;
		std::wstring sourceFile;
		ShaderType shaderType;

		bool operator==(const ShaderVariantDesc & rhs) const {
			if(entryFunctionName != rhs.entryFunctionName) {
				return false;
			}

			if(sourceFile != rhs.sourceFile) {
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

	class ShaderVariant : public Netcode::ShaderBytecode {
		ShaderVariantDesc desc;
		com_ptr<ID3DBlob> shaderByteCode;

	public:
		ShaderVariant(ShaderVariantDesc desc, com_ptr<ID3DBlob> shaderBytecode) : desc{ std::move(desc) }, shaderByteCode{ std::move(shaderBytecode) } {

		}

		const ShaderVariantDesc & GetDesc() const {
			return desc;
		}

		// Inherited via Shader
		virtual uint8_t * GetBufferPointer() override;

		virtual size_t GetBufferSize() override;

	};

	using ShaderVariantRef = std::shared_ptr<ShaderVariant>;

}