#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Decl.h"
#include <string>
#include <map>

namespace Netcode::Graphics::DX12 {
	
	com_ptr<ID3DBlob> CompileShader(const std::string & shaderSource, const std::string & entryFunction, ShaderType type);

	com_ptr<ID3DBlob> CompileShader(const std::string & shaderSource, const std::string & entryFunction, ShaderType type, const std::map<std::string, std::string> & definitions);

}
