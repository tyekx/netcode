#pragma once

#include "../../HandleTypes.h"
#include "DX12ShaderLibrary.h"

namespace Netcode::Graphics::DX12 {

	class ShaderBuilder : public Netcode::ShaderBuilder {
		enum class BuilderState {
			CLEAR, VARIANT, PRECOMPILED
		};

		Ref<DX12::ShaderLibrary> shaderLibrary;
		ShaderVariantDesc variantDesc;
		BuilderState state;

		void SetState(BuilderState st);

		void Clear();

	public:
		ShaderBuilder(Ref<DX12::ShaderLibrary> shaderLibrary);

		virtual void SetShaderType(ShaderType shaderType) override;

		virtual void SetEntrypoint(const std::string & entryFunction) override;

		virtual void SetSource(const URI::Shader & uri) override;

		virtual void SetDefinitions(const std::map<std::string, std::string> & defines) override;

		virtual Ref<Netcode::ShaderBytecode> LoadBytecode(const URI::Shader & uri) override;

		virtual Ref<Netcode::ShaderBytecode> Build() override;
	};

	using DX12ShaderBuilder = Netcode::Graphics::DX12::ShaderBuilder;
	using DX12ShaderBuilderRef = std::shared_ptr<DX12ShaderBuilder>;

}
