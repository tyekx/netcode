#include "DX12ShaderBuilder.h"

namespace Netcode::Graphics::DX12 {

	void ShaderBuilder::SetState(BuilderState st) {
		if(st == state) {
			return;
		}

		ASSERT(state == BuilderState::CLEAR || st == BuilderState::CLEAR, "Failed to set state");

		state = st;
	}

	void ShaderBuilder::Clear() {
		variantDesc = ShaderVariantDesc();
		SetState(BuilderState::CLEAR);
	}

	ShaderBuilder::ShaderBuilder(Ref<DX12::ShaderLibrary> shaderLibrary) : shaderLibrary{ std::move(shaderLibrary) }, variantDesc{}, state{ BuilderState::CLEAR } { }

	void ShaderBuilder::SetShaderType(ShaderType shaderType) {
		SetState(BuilderState::VARIANT);
		variantDesc.shaderType = shaderType;
	}

	void ShaderBuilder::SetEntrypoint(const std::string & entryFunction) {
		SetState(BuilderState::VARIANT);
		variantDesc.entryFunctionName = entryFunction;
	}

	void ShaderBuilder::SetSource(const URI::Shader & uri)
	{
		SetState(BuilderState::VARIANT);
		variantDesc.sourceFileUri = uri;
	}

	void ShaderBuilder::SetDefinitions(const std::map<std::string, std::string> & defines) {
		SetState(BuilderState::VARIANT);
		variantDesc.defines = defines;
	}

	Ref<Netcode::ShaderBytecode> ShaderBuilder::LoadBytecode(const URI::Shader & uri) {
		SetState(BuilderState::PRECOMPILED);
		auto shader = shaderLibrary->LoadShader(uri);
		Clear();
		return shader;
	}

	Ref<Netcode::ShaderBytecode> ShaderBuilder::Build() {
		ASSERT(state == BuilderState::VARIANT, "Failed to build shader: no inputs were given");

		if(state == BuilderState::VARIANT) {
			auto variant = shaderLibrary->GetShaderVariant(variantDesc);
			Clear();
			return variant;
		} else {
			return nullptr;
		}
	}

}


