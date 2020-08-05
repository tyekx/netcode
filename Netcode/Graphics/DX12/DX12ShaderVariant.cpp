#include "../../Common.h"
#include "DX12ShaderVariant.h"
#include "DX12ShaderCompiler.h"
#include <d3dcompiler.h>

namespace Netcode::Graphics::DX12 {
	ShaderVariant::ShaderVariant(ShaderVariantDesc desc, com_ptr<ID3DBlob> shaderBytecode) : desc{ std::move(desc) }, shaderByteCode{ std::move(shaderBytecode) } {

	}


	const ShaderVariantDesc & ShaderVariant::GetDesc() const {
		return desc;
	}

	uint8_t * ShaderVariant::GetBufferPointer()
	{
		return reinterpret_cast<uint8_t *>(shaderByteCode->GetBufferPointer());
	}

	size_t ShaderVariant::GetBufferSize()
	{
		return shaderByteCode->GetBufferSize();
	}
}

