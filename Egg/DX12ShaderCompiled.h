#pragma once

#include "Shader.h"

namespace Egg::Graphics::DX12 {

	class ShaderCompiled : public Egg::Graphics::Shader {
		com_ptr<ID3DBlob> bufferBlob;

	public:
		// Inherited via Shader
		virtual uint8_t * GetBufferPointer() override;
		virtual size_t GetBufferSize() override;

		virtual void SetEntryFunction(const std::string & functionName) override;

		virtual void SetShaderSource(const std::string & sourceCode) override;

		virtual void SetDefinitions(const std::map<std::string, std::string> & defs) override;

		ShaderCompiled(const std::wstring & sourceFile, const void * srcData, size_t size);
		
		ShaderCompiled(const std::wstring & sourceFile, com_ptr<ID3DBlob> blob);

		static std::unique_ptr<ShaderCompiled> LoadCSO(const std::wstring & absolutePath);
	};

}
