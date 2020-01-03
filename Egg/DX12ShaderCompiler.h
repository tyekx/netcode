#pragma once

#include "DX12Common.h"
#include "Shader.h"

namespace Egg::Graphics::DX12 {
	
	com_ptr<ID3DBlob> CompileShader(const std::string & shaderSource, const std::string & entryFunction, ShaderType type);

	com_ptr<ID3DBlob> CompileShader(const std::string & shaderSource, const std::string & entryFunction, ShaderType type, const std::map<std::string, std::string> & definitions);

}
