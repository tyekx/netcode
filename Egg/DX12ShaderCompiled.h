#pragma once

#include "HandleTypes.h"
#include "DX12Common.h"

namespace Egg::Graphics::DX12 {

	class ShaderCompiled : public Egg::ShaderBytecode {
		com_ptr<ID3DBlob> bufferBlob;
		std::wstring sourceFileRef;

	public:
		// Inherited via Shader
		virtual uint8_t * GetBufferPointer() override;
		virtual size_t GetBufferSize() override;

		const std::wstring & GetFileReference() const {
			return sourceFileRef;
		}

		ShaderCompiled(const std::wstring & sourceFile, const void * srcData, size_t size);
		
		ShaderCompiled(const std::wstring & sourceFile, com_ptr<ID3DBlob> blob);
	};
	
	using ShaderCompiledRef = std::shared_ptr<ShaderCompiled>;

}
