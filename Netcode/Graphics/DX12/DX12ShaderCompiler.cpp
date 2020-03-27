#include "../../HandleTypes.h"
#include "../../Utility.h"

#include "DX12ShaderCompiler.h"

#include <memory>
#include <map>

namespace Egg::Graphics::DX12 {

	static const char * GetShaderVersion(ShaderType type) {
		switch(type) {
			case ShaderType::VERTEX_SHADER: return "vs_5_0";
			case ShaderType::PIXEL_SHADER: return "ps_5_0";
			case ShaderType::GEOMETRY_SHADER: return "gs_5_0";
			case ShaderType::HULL_SHADER: return "hs_5_0";
			case ShaderType::DOMAIN_SHADER: return "ds_5_0";
			default:
				return "";
		}
	}

	com_ptr<ID3DBlob> CompileShader(const std::string & shaderSource, const std::string & entryFunction, ShaderType type) {
		std::map<std::string, std::string> nodefs;
		return CompileShader(shaderSource, entryFunction, type, nodefs);
	}

	com_ptr<ID3DBlob> CompileShader(const std::string & shaderSource, const std::string & entryFunction, ShaderType type, const std::map<std::string, std::string> & definitions) {
		com_ptr<ID3DBlob> byteCode;

		std::unique_ptr<D3D_SHADER_MACRO[]> preprocDefinitions{ nullptr };

		if(definitions.size() > 0) {
			preprocDefinitions = std::make_unique<D3D_SHADER_MACRO[]>(definitions.size() + 1);

			int preprocIt = 0;
			for(const auto & kv : definitions) {
				preprocDefinitions[preprocIt].Name = kv.first.c_str();
				preprocDefinitions[preprocIt].Definition = (kv.second.size() > 0) ? kv.second.c_str() : nullptr;
				++preprocIt;
			}
			// "NULL" termination, API requires this, dont delete
			preprocDefinitions[preprocIt].Name = nullptr;
			preprocDefinitions[preprocIt].Definition = nullptr;
		}

		com_ptr<ID3DBlob> errorMsg;

		D3DCompile(shaderSource.c_str(), shaderSource.size(), nullptr, preprocDefinitions.get(), nullptr, entryFunction.c_str(), GetShaderVersion(type), 0, 0, byteCode.GetAddressOf(), errorMsg.GetAddressOf());

		if(errorMsg != nullptr) {
			Egg::Utility::Debugf("Error while compiling vertex shader (-Wall): \r\n");
			DebugPrintBlob(errorMsg);
		}

		return byteCode;
	}

}
