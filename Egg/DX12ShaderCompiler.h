#pragma once

#include "Common.h"
#include "DX12ShaderVariant.h"
#include "DX12ShaderBytecode.h"

namespace Egg::Graphics::DX12 {

	class ShaderCompiler {

	public:

		// @TODO: could be static
		ShaderBytecode Compile(const ShaderVariant & variant) {

			com_ptr<ID3DBlob> vsByteCode;
			const auto & pd = variant.GetPreprocDefs();

			std::unique_ptr<D3D_SHADER_MACRO[]> preprocDefinitions{ nullptr };

			if(pd.defs.size() > 0) {
				preprocDefinitions = std::make_unique<D3D_SHADER_MACRO[]>(pd.defs.size() + 1);

				int preprocIt = 0;
				for(const auto & kv : pd.defs) {
					preprocDefinitions[preprocIt].Name = kv.first.c_str();
					preprocDefinitions[preprocIt].Definition = (kv.second.size() > 0) ? kv.second.c_str() : nullptr;
					++preprocIt;
				}
				// "NULL" termination, API requires this, dont delete
				preprocDefinitions[preprocIt].Name = nullptr;
				preprocDefinitions[preprocIt].Definition = nullptr;
			}

			const char * entryPoint = variant.GetEntryFunction().c_str();

			com_ptr<ID3DBlob> errorMsg;

			D3DCompile(variant.GetSource().c_str(), variant.GetSource().size(), nullptr, preprocDefinitions.get(), nullptr, entryPoint, "vs_5_0", 0, 0, vsByteCode.GetAddressOf(), errorMsg.GetAddressOf());

			if(errorMsg != nullptr) {
				Egg::Utility::Debugf("Error while compiling vertex shader (-Wall): \r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
			}

			return ShaderBytecode{ variant.GetFileReference(), nullptr, variant.GetPreprocDefs(), variant.GetShaderType() };
		}



	};

}
