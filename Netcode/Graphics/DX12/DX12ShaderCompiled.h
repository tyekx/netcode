#pragma once

#include <Netcode/HandleTypes.h>
#include "DX12Decl.h"
#include <wrl.h>

namespace Netcode::Graphics::DX12 {

	class ShaderCompiled : public Netcode::ShaderBytecode {
		com_ptr<ID3DBlob> bufferBlob;
		std::wstring sourceFileRef;

	public:
		virtual uint8_t * GetBufferPointer() override;
		virtual size_t GetBufferSize() override;

		virtual const std::wstring & GetFileReference() const override;

		ShaderCompiled(std::wstring sourceFile, const void * srcData, size_t size);
		
		ShaderCompiled(std::wstring sourceFile, com_ptr<ID3DBlob> blob);
	};

}
