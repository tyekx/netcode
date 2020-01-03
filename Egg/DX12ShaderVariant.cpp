#include "DX12ShaderVariant.h"
#include "DX12ShaderCompiler.h"

namespace Egg::Graphics::DX12 {

	
	void ShaderVariant::SetShaderType(ShaderType type) {
		shaderType = type;
	}
	
	void ShaderVariant::SetSourceReference(const std::wstring & shader) {
		sourceFile = shader;
	}
	
	void ShaderVariant::SetShaderSource(const std::string & shaderSource) {
		sourceCode = shaderSource;
	}
	
	void ShaderVariant::SetDefinitions(const std::map<std::string, std::string> & macros) {
		defs = ShaderPreprocDefs{ macros };
	}
	
	void ShaderVariant::SetEntryFunction(const std::string & fname) {
		entryFunctionName = fname;
	}
	
	ShaderType ShaderVariant::GetShaderType() const {
		return shaderType;
	}
	
	const std::wstring & ShaderVariant::GetFileReference() const {
		return sourceFile;
	}
	
	const std::string & ShaderVariant::GetEntryFunction() const {
		return entryFunctionName;
	}
	
	const std::string & ShaderVariant::GetSource() const {
		return sourceCode;
	}
	
	const ShaderPreprocDefs & ShaderVariant::GetPreprocDefs() const {
		return defs;
	}

	uint8_t * ShaderVariant::GetBufferPointer()
	{
		if(shaderByteCode == nullptr) {
			shaderByteCode = CompileShader(sourceCode, entryFunctionName, shaderType, defs.defs);
		}

		return reinterpret_cast<uint8_t *>(shaderByteCode->GetBufferPointer());
	}

	size_t ShaderVariant::GetBufferSize()
	{
		if(shaderByteCode == nullptr) {
			shaderByteCode = CompileShader(sourceCode, entryFunctionName, shaderType, defs.defs);
		}

		return shaderByteCode->GetBufferSize();
	}
}

