#pragma once

#include "Shader.h"
#include "HandleTypes.h"
#include "GraphicsContexts.h"
#include "DX12ShaderSourceLibrary.h"

namespace Egg::Graphics::DX12 {

	class ShaderContext : public Egg::Graphics::IShaderContext {

		std::vector<std::unique_ptr<Shader>> shaders;

		ShaderSourceLibrary sourceLibrary;

	public:

		HSHADER Insert(std::unique_ptr<Shader> shader);

		Shader * GetShader(HSHADER handle);

		// Inherited via IShaderContext
		virtual HSHADER Load(const std::wstring & shaderPath) override;

		virtual HSHADER CreateVertexShader() override;

		virtual HSHADER CreatePixelShader() override;

		virtual HSHADER CreateGeometryShader() override;

		virtual HSHADER CreateDomainShader() override;

		virtual HSHADER CreateHullShader() override;

		virtual void SetEntrypoint(HSHADER shader, const std::string & entryFunction) override;

		virtual void SetSource(HSHADER shader, const std::wstring & shaderPath) override;

		virtual void SetDefinitions(HSHADER shader, const std::map<std::string, std::string> & defines) override;

	};

}
