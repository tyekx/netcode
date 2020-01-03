#include "DX12ShaderContext.h"

namespace Egg::Graphics::DX12 {
	HSHADER ShaderContext::Insert(std::unique_ptr<Shader> shader) {
		HSHADER handle = static_cast<HSHADER>(shaders.size());

		shaders.emplace_back(std::move(shader));

		return handle;
	}
	
	Shader * ShaderContext::GetShader(HSHADER handle) {
		ASSERT(handle < static_cast<UINT>(shaders.size()), "handle is out of range");

		return shaders[handle].get();
	}

	HSHADER ShaderContext::Load(const std::wstring & shaderPath) {
		return HSHADER();
	}

	HSHADER ShaderContext::CreateVertexShader() {
		return HSHADER();
	}

	HSHADER ShaderContext::CreatePixelShader() {
		return HSHADER();
	}

	HSHADER ShaderContext::CreateGeometryShader() {
		return HSHADER();
	}

	HSHADER ShaderContext::CreateDomainShader() {
		return HSHADER();
	}

	HSHADER ShaderContext::CreateHullShader() {
		return HSHADER();
	}

	void ShaderContext::SetEntrypoint(HSHADER shader, const std::string & entryFunction) {
		GetShader(shader)->SetEntryFunction(entryFunction);
	}

	void ShaderContext::SetSource(HSHADER shader, const std::wstring & shaderPath) {
		GetShader(shader)->SetShaderSource(sourceLibrary.GetSource(shaderPath));
	}

	void ShaderContext::SetDefinitions(HSHADER shader, const std::map<std::string, std::string> & defines) {
		GetShader(shader)->SetDefinitions(defines);
	}

}
