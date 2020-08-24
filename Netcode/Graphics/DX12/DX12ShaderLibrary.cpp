#include "DX12ShaderLibrary.h"
#include <Netcode/Common.h>
#include <Netcode/Utility.h>
#include <Netcode/URI/Shader.h>
#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>
#include "DX12Common.h"
#include "DX12ShaderCompiler.h"
#include "DX12ShaderCompiled.h"
#include "DX12ShaderVariant.h"
#include "DX12Decl.h"
#include "DX12Includes.h"
#include <d3dcompiler.h>

namespace Netcode::Graphics::DX12 {

	Ref<DX12::ShaderCompiled> ShaderLibrary::LoadCSO(const std::wstring & absolutePath) {
		IO::File shaderFile{ absolutePath };
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

	Ref<DX12::ShaderCompiled> ShaderLibrary::LoadShader(const URI::Shader & filePath) {
#if defined(NETCODE_EDITOR_VARIANT)
		Log::Debug("[DX12] Loading shader object: {0}", Netcode::Utility::ToNarrowString(filePath.GetFullPath()));

		Ref<ShaderCompiled> compiledRef = LoadCSO(filePath.GetShaderPath());

		ASSERT(compiledRef != nullptr, "Failed to load cso");

		return compiledRef;
#else 
		auto it = std::find_if(std::begin(compiledShaders), std::end(compiledShaders), [&filePath](const Ref<DX12::ShaderCompiled> & ref) -> bool {
			if(ref->GetFileReference() == filePath.GetShaderPath()) {
				return true;
			}
			return false;
		});

		if(it == std::end(compiledShaders)) {
			Log::Debug("[DX12] Loading shader object: {0}", Netcode::Utility::ToNarrowString(filePath.GetFullPath()));

			Ref<ShaderCompiled> compiledRef = LoadCSO(filePath.GetShaderPath());

			ASSERT(compiledRef != nullptr, "Failed to load cso");

			compiledShaders.push_back(compiledRef);

			return compiledRef;
		} else {
			return *it;
		}
#endif
	}
	
	Ref<DX12::ShaderVariant>  ShaderLibrary::GetShaderVariant(const ShaderVariantDesc & desc) {
		auto it = std::find_if(std::begin(shaderVariants), std::end(shaderVariants), [&desc](const Ref<ShaderVariant> & ref) -> bool {
			if(ref->GetDesc() == desc) {
				return true;
			}
			return false;
		});

		if(it == std::end(shaderVariants)) {
			com_ptr<ID3DBlob> blob = CompileShader(shaderSources.GetSource(desc.sourceFileUri.GetFullPath()), desc.entryFunctionName, desc.shaderType, desc.defines);

			Ref<DX12::ShaderVariant> shaderVariant = std::make_shared<DX12::ShaderVariant>(desc, std::move(blob));

			shaderVariants.push_back(shaderVariant);

			return shaderVariant;
		} else {
			return *it;
		}
	}
}
