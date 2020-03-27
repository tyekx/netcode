#include "../../Common.h"
#include "DX12ShaderVariant.h"
#include "DX12ShaderCompiler.h"

namespace Netcode::Graphics::DX12 {

	uint8_t * ShaderVariant::GetBufferPointer()
	{
		return reinterpret_cast<uint8_t *>(shaderByteCode->GetBufferPointer());
	}

	size_t ShaderVariant::GetBufferSize()
	{
		return shaderByteCode->GetBufferSize();
	}
}

