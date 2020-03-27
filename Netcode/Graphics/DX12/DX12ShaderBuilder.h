#pragma once

#include "../../HandleTypes.h"
#include "DX12ShaderLibrary.h"

namespace Egg::Graphics::DX12 {

	class ShaderBuilder : public Egg::ShaderBuilder {
		enum class BuilderState {
			CLEAR, VARIANT, PRECOMPILED
		};

		DX12ShaderLibraryRef shaderLibrary;
		ShaderVariantDesc variantDesc;
		BuilderState state;

		void SetState(BuilderState st);

		void Clear();

	public:
		ShaderBuilder(DX12ShaderLibraryRef shaderLibrary);

		virtual void SetShaderType(ShaderType shaderType) override;

		virtual void SetEntrypoint(const std::string & entryFunction) override;

		virtual void SetSource(const std::wstring & shaderPath) override;

		virtual void SetDefinitions(const std::map<std::string, std::string> & defines) override;

		virtual ShaderBytecodeRef LoadBytecode(const std::wstring & precompiledShaderPath) override;

		virtual ShaderBytecodeRef Build() override;
	};

	using DX12ShaderBuilder = Egg::Graphics::DX12::ShaderBuilder;
	using DX12ShaderBuilderRef = std::shared_ptr<DX12ShaderBuilder>;

}
