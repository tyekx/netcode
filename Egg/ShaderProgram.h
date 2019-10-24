#pragma once

#include "Common.h"
#include <fstream>
#include "Path.h"

namespace Egg {
	
	class ShaderProgram {
		com_ptr<ID3DBlob> byteCode;

	public:

		ShaderProgram(com_ptr<ID3DBlob> && blobRvalue);
		
		D3D12_SHADER_BYTECODE GetByteCode() const;

		static com_ptr<ID3D12RootSignature> LoadRootSignature(ID3D12Device * device, const ShaderPath & filename);

		static com_ptr<ID3D12RootSignature> LoadRootSignature(ID3D12Device * device, ID3DBlob * blobWithRootSignature);

		static com_ptr<ID3DBlob> LoadCso(const ShaderPath & absolutePath);

	};

}
