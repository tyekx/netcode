#include "DX12ShaderBuilder.h"
#include <Netcode/Common.h>
#include "DX12Common.h"
#include "DX12ShaderCompiled.h"
#include "DX12ShaderVariant.h"
#include "DX12ShaderLibrary.h"

namespace Netcode::Graphics::DX12 {
	void ShaderBuilderImpl::InitVariantDescIfEmpty()
	{
		if(variantDesc == nullptr) {
			variantDesc = std::make_unique<ShaderVariantDesc>();
		}
	}

	void ShaderBuilderImpl::Clear() {
		variantDesc.reset();
	}

	ShaderBuilderImpl::ShaderBuilderImpl(Ref<ShaderLibrary> shaderLibrary) : shaderLibrary{ std::move(shaderLibrary) }, variantDesc{} { }

	void ShaderBuilderImpl::SetShaderType(ShaderType shaderType) {
		InitVariantDescIfEmpty();
		variantDesc->shaderType = shaderType;
	}

	void ShaderBuilderImpl::SetEntrypoint(const std::string & entryFunction) {
		InitVariantDescIfEmpty();
		variantDesc->entryFunctionName = entryFunction;
	}

	void ShaderBuilderImpl::SetSource(const URI::Shader & uri)
	{
		InitVariantDescIfEmpty();
		variantDesc->sourceFileUri = uri;
	}

	void ShaderBuilderImpl::SetDefinitions(const std::map<std::string, std::string> & defines) {
		InitVariantDescIfEmpty();
		variantDesc->defines = defines;
	}

	Ref<ShaderBytecode> ShaderBuilderImpl::LoadBytecode(const URI::Shader & uri) {
		auto shader = shaderLibrary->LoadShader(uri);
		Clear();
		return shader;
	}

	Ref<ShaderBytecode> ShaderBuilderImpl::Build() {
		ASSERT(variantDesc != nullptr, "Failed to build shader: no inputs were given");
		auto variant = shaderLibrary->GetShaderVariant(*variantDesc);
		Clear();
		return variant;
	}

}


