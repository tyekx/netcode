#pragma once

#include <Netcode/HandleTypes.h>
#include <memory>

namespace Netcode::Graphics::DX12 {

	class ShaderLibrary;
	struct ShaderVariantDesc;

	class ShaderBuilderImpl : public Netcode::ShaderBuilder {
		Ref<ShaderLibrary> shaderLibrary;
		Unique<ShaderVariantDesc> variantDesc;

		void InitVariantDescIfEmpty();

		void Clear();

	public:
		ShaderBuilderImpl(Ref<ShaderLibrary> shaderLibrary);

		virtual void SetShaderType(ShaderType shaderType) override;

		virtual void SetEntrypoint(const std::string & entryFunction) override;

		virtual void SetSource(const URI::Shader & uri) override;

		virtual void SetDefinitions(const std::map<std::string, std::string> & defines) override;

		virtual Ref<ShaderBytecode> LoadBytecode(const URI::Shader & uri) override;

		virtual Ref<ShaderBytecode> Build() override;
	};

}
