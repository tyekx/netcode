#pragma once

#include "Common.h"
#include "Utility.h"
#include "PreprocessorDefinitions.h"
#include "ShaderCodeCollection.h"
#include "Path.h"

namespace Egg::Graphics::Internal {

	class ShaderCodeLibrary {

		bool Exists(const PreprocessorDefinitions & pd) {
			return false;
		}

		void CompileWith(const PreprocessorDefinitions & pd) {
			com_ptr<ID3DBlob> vsByteCode;
			com_ptr<ID3DBlob> psByteCode;

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

			const char * vertexShaderEntry = "Vertex_Main";
			const char * pixelShaderEntry = "Pixel_Main";

			std::string file;
			ShaderPath path{ L"EggShaderLib.hlsli" };
			Egg::Utility::SlurpFile(file, L"C:/work/directx12/Egg/EggShaderLib.hlsli");
			com_ptr<ID3DBlob> errorMsg;

			D3DCompile(file.c_str(), file.size(), nullptr, preprocDefinitions.get(), nullptr, vertexShaderEntry, "vs_5_0", 0, 0, vsByteCode.GetAddressOf(), errorMsg.GetAddressOf());

			if(errorMsg != nullptr) {
				Egg::Utility::Debugf("Error while compiling vertex shader (all warnings are treated as errors): \r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
				return;
			}

			D3DCompile(file.c_str(), file.size(), nullptr, preprocDefinitions.get(), nullptr, pixelShaderEntry, "ps_5_0", 0, 0, psByteCode.GetAddressOf(), errorMsg.GetAddressOf());

			if(errorMsg != nullptr) {
				Egg::Utility::Debugf("Error while compiling pixel shader (all warnings are treated as errors): \r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
				return;
			}
		}

		ShaderCodeCollection FindCollection(const PreprocessorDefinitions & pd) {
			return ShaderCodeCollection{};
		}

	public:
		ShaderCodeCollection GetShaderCodeCollection(const PreprocessorDefinitions & pd) {
			if(!Exists(pd)) {
				CompileWith(pd);
			}

			return FindCollection(pd);
		}
	};

}


