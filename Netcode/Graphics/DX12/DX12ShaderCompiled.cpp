#include "../../Common.h"
#include "../../Logger.h"
#include "DX12Common.h"
#include "DX12ShaderCompiled.h"
#include <fstream>

namespace Egg::Graphics::DX12 {
	ShaderCompiled::ShaderCompiled(const std::wstring & sourceFile, const void * srcData, size_t size) : bufferBlob{ nullptr }, sourceFileRef{ sourceFile } {

		DX_API("Failed to create blob")
			D3DCreateBlob(size, bufferBlob.GetAddressOf());

		memcpy(bufferBlob->GetBufferPointer(), srcData, size);
	}

	ShaderCompiled::ShaderCompiled(const std::wstring & sourceFile, com_ptr<ID3DBlob> blob) : bufferBlob{ std::move(blob) }, sourceFileRef{ sourceFile } {
	}

	uint8_t * ShaderCompiled::GetBufferPointer() {
		return reinterpret_cast<uint8_t *>((bufferBlob != nullptr) ? bufferBlob->GetBufferPointer() : nullptr);
	}

	size_t ShaderCompiled::GetBufferSize() {
		return (bufferBlob != nullptr) ? bufferBlob->GetBufferSize() : 0;
	}

}
