#pragma once

#include "../../Common.h"
#include "DX12Common.h"
#include "DX12ShaderVariant.h"
#include "DX12ShaderCompiled.h"
#include "DX12ShaderCompiler.h"
#include "DX12ShaderSourceLibrary.h"

namespace Netcode::Graphics::DX12 {

	class ShaderLibrary {
		std::vector<Ref<DX12::ShaderVariant>> shaderVariants;
		std::vector<Ref<DX12::ShaderCompiled>> compiledShaders;
		ShaderSourceLibrary shaderSources;
		
		static Ref<DX12::ShaderCompiled> LoadCSO(std::wstring_view absolutePath);

	public:
		Ref<DX12::ShaderCompiled> LoadShader(const URI::Shader & filePath);
		Ref<DX12::ShaderVariant> GetShaderVariant(const ShaderVariantDesc & desc);
	};

}
