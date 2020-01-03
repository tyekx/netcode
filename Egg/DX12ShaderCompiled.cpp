#include "DX12Common.h"
#include "DX12ShaderCompiled.h"
#include "Common.h"
#include <fstream>
#include "Logger.h"

namespace Egg::Graphics::DX12 {
	ShaderCompiled::ShaderCompiled(const std::wstring & sourceFile, const void * srcData, size_t size) : bufferBlob{ nullptr } {
		Shader::sourceFile = sourceFile;

		DX_API("Failed to create blob")
			D3DCreateBlob(size, bufferBlob.GetAddressOf());

		memcpy(bufferBlob->GetBufferPointer(), srcData, size);
	}

	ShaderCompiled::ShaderCompiled(const std::wstring & sourceFile, com_ptr<ID3DBlob> blob) : bufferBlob{ nullptr } {
		Shader::sourceFile = sourceFile;
		std::swap(blob, bufferBlob);
	}

	std::unique_ptr<ShaderCompiled> ShaderCompiled::LoadCSO(const std::wstring & absolutePath) {
		std::ifstream file{ absolutePath, std::ios::binary | std::ios::ate };

		ASSERT(file.is_open(), "Failed to open blob file: '%S'", absolutePath.c_str());

		std::streamsize size = file.tellg();

		file.seekg(0, std::ios::beg);

		com_ptr<ID3DBlob> shaderByteCode{ nullptr };

		DX_API("Failed to allocate memory for blob")
			D3DCreateBlob(static_cast<size_t>(size), shaderByteCode.GetAddressOf());

		if(file.read(reinterpret_cast<char *>(shaderByteCode->GetBufferPointer()), size)) {
			return std::make_unique<ShaderCompiled>(absolutePath, std::move(shaderByteCode));
		} else {
			Log::Error("Failed to read from shader");
			return nullptr;
		}
	}

	uint8_t * ShaderCompiled::GetBufferPointer() {
		return reinterpret_cast<uint8_t *>((bufferBlob != nullptr) ? bufferBlob->GetBufferPointer() : nullptr);
	}

	size_t ShaderCompiled::GetBufferSize() {
		return (bufferBlob != nullptr) ? bufferBlob->GetBufferSize() : 0;
	}

	void ShaderCompiled::SetShaderSource(const std::string & sourceCode) {
		Log::Warn("ignoring SetShaderSource on a compiled shader");
	}

	void ShaderCompiled::SetDefinitions(const std::map<std::string, std::string> & defs) {
		Log::Warn("ignoring SetDefinitions on a compiled shader");
	}

	void ShaderCompiled::SetEntryFunction(const std::string & functionName) {
		Log::Warn("ignoring SetEntryFunction on a compiled shader");
	}

}
