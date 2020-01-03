#include "DX12ShaderContext.h"
#include "DX12ShaderVariant.h"

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
		UINT id = static_cast<UINT>(shaders.size());
		std::unique_ptr<ShaderVariant> variant = std::make_unique<ShaderVariant>();
		variant->SetShaderType(ShaderType::VERTEX_SHADER);
		shaders.emplace_back(std::move(variant));
		return id;
	}

	HSHADER ShaderContext::CreatePixelShader() {
		UINT id = static_cast<UINT>(shaders.size());
		std::unique_ptr<ShaderVariant> variant = std::make_unique<ShaderVariant>();
		variant->SetShaderType(ShaderType::PIXEL_SHADER);
		shaders.emplace_back(std::move(variant));
		return id;
	}

	HSHADER ShaderContext::CreateGeometryShader() {
		UINT id = static_cast<UINT>(shaders.size());
		std::unique_ptr<ShaderVariant> variant = std::make_unique<ShaderVariant>();
		variant->SetShaderType(ShaderType::GEOMETRY_SHADER);
		shaders.emplace_back(std::move(variant));
		return id;
	}

	HSHADER ShaderContext::CreateDomainShader() {
		UINT id = static_cast<UINT>(shaders.size());
		std::unique_ptr<ShaderVariant> variant = std::make_unique<ShaderVariant>();
		variant->SetShaderType(ShaderType::DOMAIN_SHADER);
		shaders.emplace_back(std::move(variant));
		return id;
	}

	HSHADER ShaderContext::CreateHullShader() {
		UINT id = static_cast<UINT>(shaders.size());
		std::unique_ptr<ShaderVariant> variant = std::make_unique<ShaderVariant>();
		variant->SetShaderType(ShaderType::HULL_SHADER);
		shaders.emplace_back(std::move(variant));
		return id;
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
