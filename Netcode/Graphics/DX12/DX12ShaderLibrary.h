#pragma once

#include "../../Common.h"
#include "DX12Common.h"
#include "DX12ShaderVariant.h"
#include "DX12ShaderCompiled.h"
#include "DX12ShaderCompiler.h"
#include "DX12ShaderSourceLibrary.h"

namespace Netcode::Graphics::DX12 {

	class ShaderLibrary {
		std::vector<ShaderVariantRef> shaderVariants;
		std::vector<ShaderCompiledRef> compiledShaders;
		ShaderSourceLibrary shaderSources;
		
		static ShaderCompiledRef LoadCSO(std::wstring_view absolutePath);

	public:
		ShaderCompiledRef LoadShader(const URI::Shader & filePath);

		ShaderVariantRef GetShaderVariant(const ShaderVariantDesc & desc);
	};

	using DX12ShaderLibrary = Netcode::Graphics::DX12::ShaderLibrary;
	using DX12ShaderLibraryRef = std::shared_ptr<DX12ShaderLibrary>;

}
