#include "ShaderProgram.h"
#include "Utility.h"

Egg::ShaderProgram::ShaderProgram(com_ptr<ID3DBlob> && blobRvalue) : byteCode{ std::move(blobRvalue) } { }

D3D12_SHADER_BYTECODE Egg::ShaderProgram::GetByteCode() const {
	D3D12_SHADER_BYTECODE bc;
	bc.BytecodeLength = byteCode->GetBufferSize();
	bc.pShaderBytecode = byteCode->GetBufferPointer();
	return bc;
}

com_ptr<ID3D12RootSignature> Egg::ShaderProgram::LoadRootSignature(ID3D12Device * device, const ShaderPath & filename) {
	com_ptr<ID3DBlob> rootSigBlob = LoadCso(filename.GetAbsolutePath());

	return LoadRootSignature(device, rootSigBlob.Get());
}

com_ptr<ID3D12RootSignature> Egg::ShaderProgram::LoadRootSignature(ID3D12Device * device, ID3DBlob * blobWithRootSignature) {
	com_ptr<ID3D12RootSignature> rootSig{ nullptr };

	DX_API("Failed to create root signature")
		device->CreateRootSignature(0, blobWithRootSignature->GetBufferPointer(),
									blobWithRootSignature->GetBufferSize(), IID_PPV_ARGS(rootSig.GetAddressOf()));

	return rootSig;
}

com_ptr<ID3DBlob> Egg::ShaderProgram::LoadCso(const ShaderPath & shaderPath) {
	std::wstring filename = shaderPath.GetAbsolutePath();

	std::ifstream file{ filename.c_str(), std::ios::binary | std::ios::in | std::ios::ate };

	ASSERT(file.is_open(), "Failed to open blob file: %S", filename.c_str());

	std::streamsize size = file.tellg();

	file.seekg(0, std::ios::beg);

	com_ptr<ID3DBlob> shaderByteCode{ nullptr };

	DX_API("Failed to allocate memory for blob")
		D3DCreateBlob((size_t)size, shaderByteCode.GetAddressOf());

	if(file.read(reinterpret_cast<char*>(shaderByteCode->GetBufferPointer()), size)) {
		return shaderByteCode;
	} else {
		ASSERT(false, "Failed to load CSO file: %S", filename.c_str());
		return nullptr;
	}
}
