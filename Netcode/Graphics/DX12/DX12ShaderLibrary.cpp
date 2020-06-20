#include "DX12ShaderLibrary.h"

#include "../../Utility.h"
#include "../../IO/File.h"
#include "../../IO/Path.h"

namespace Netcode::Graphics::DX12 {

	ShaderCompiledRef ShaderLibrary::LoadCSO(std::wstring_view absolutePath) {
		IO::File shaderFile{ std::wstring{ absolutePath } };
		IO::FileReader<IO::File> reader{ shaderFile, Netcode::IO::FileOpenMode::READ_BINARY };

		size_t numBytes = reader->GetSize();

		com_ptr<ID3DBlob> shaderByteCode{ nullptr };

		DX_API("Failed to allocate memory for blob")
			D3DCreateBlob(static_cast<size_t>(numBytes), shaderByteCode.GetAddressOf());

		size_t readBytes = reader->Read(MutableArrayView<uint8_t>{
			reinterpret_cast<uint8_t*>(shaderByteCode->GetBufferPointer()),
			shaderByteCode->GetBufferSize()
		});

		UndefinedBehaviourAssertion(readBytes == shaderByteCode->GetBufferSize() && readBytes == numBytes);

		return std::make_shared<ShaderCompiled>(std::wstring{ absolutePath }, std::move(shaderByteCode));
	}

	ShaderCompiledRef ShaderLibrary::LoadShader(const std::wstring & filePath) {
		IO::File tmpFile{ IO::Path::ShaderRoot(), filePath };

		auto it = std::find_if(std::begin(compiledShaders), std::end(compiledShaders), [&filePath](const ShaderCompiledRef & ref) -> bool {
			if(ref->GetFileReference() == filePath) {
				return true;
			}
			return false;
		});

		if(it == std::end(compiledShaders)) {
			Log::Debug("[DX12] Loading shader object: {0}", Netcode::Utility::ToNarrowString(filePath));

			ShaderCompiledRef compiledRef = LoadCSO(tmpFile.GetFullPath());

			ASSERT(compiledRef != nullptr, "Failed to load cso");

			compiledShaders.push_back(compiledRef);

			return compiledRef;
		} else {
			return *it;
		}
	}
	
	ShaderVariantRef ShaderLibrary::GetShaderVariant(const ShaderVariantDesc & desc) {
		auto it = std::find_if(std::begin(shaderVariants), std::end(shaderVariants), [&desc](const ShaderVariantRef & ref) -> bool {
			if(ref->GetDesc() == desc) {
				return true;
			}
			return false;
		});

		if(it == std::end(shaderVariants)) {
			com_ptr<ID3DBlob> blob = CompileShader(shaderSources.GetSource(desc.sourceFile), desc.entryFunctionName, desc.shaderType, desc.defines);

			ShaderVariantRef shaderVariant = std::make_shared<ShaderVariant>(desc, std::move(blob));

			shaderVariants.push_back(shaderVariant);

			return shaderVariant;
		} else {
			return *it;
		}
	}
}
