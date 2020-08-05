#pragma once

#include <Netcode/HandleDecl.h>
#include <vector>
#include "DX12ShaderSourceLibrary.h"


namespace Netcode::URI {

	class Shader;

}

namespace Netcode::Graphics::DX12 {

	struct ShaderVariantDesc;
	class ShaderVariant;
	class ShaderCompiled;

	class ShaderLibrary {
		std::vector<Ref<ShaderVariant>> shaderVariants;
		std::vector<Ref<ShaderCompiled>> compiledShaders;
		ShaderSourceLibrary shaderSources;
		
		static Ref<ShaderCompiled> LoadCSO(const std::wstring & absolutePath);

	public:
		Ref<ShaderCompiled> LoadShader(const URI::Shader & filePath);
		Ref<ShaderVariant> GetShaderVariant(const ShaderVariantDesc & desc);
	};

}
