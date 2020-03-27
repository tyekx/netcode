#pragma once

#include "../../Common.h"
#include "DX12Common.h"
#include "DX12ShaderVariant.h"
#include "DX12ShaderCompiled.h"
#include "DX12ShaderCompiler.h"
#include <fstream>
#include "DX12ShaderSourceLibrary.h"

namespace Netcode::Graphics::DX12 {

	class ShaderLibrary {
		std::vector<ShaderVariantRef> shaderVariants;
		std::vector<ShaderCompiledRef> compiledShaders;
		ShaderSourceLibrary shaderSources;

		static ShaderCompiledRef LoadCSO(const std::wstring & absolutePath) {
			std::ifstream file{ absolutePath, std::ios::binary | std::ios::ate };

			ASSERT(file.is_open(), "Failed to open blob file: '%S'", absolutePath.c_str());

			std::streamsize size = file.tellg();

			file.seekg(0, std::ios::beg);

			com_ptr<ID3DBlob> shaderByteCode{ nullptr };

			DX_API("Failed to allocate memory for blob")
				D3DCreateBlob(static_cast<size_t>(size), shaderByteCode.GetAddressOf());

			if(file.read(reinterpret_cast<char *>(shaderByteCode->GetBufferPointer()), size)) {
				return std::make_shared<ShaderCompiled>(absolutePath, std::move(shaderByteCode));
			} else {
				Log::Error("Failed to read from shader");
				return nullptr;
			}
		}

	public:

		ShaderCompiledRef LoadShader(const std::wstring & filePath) {
			auto it = std::find_if(std::begin(compiledShaders), std::end(compiledShaders), [&filePath](const ShaderCompiledRef & ref) -> bool {
				if(ref->GetFileReference() == filePath) {
					return true;
				}
				return false;
			});

			if(it == std::end(compiledShaders)) {
				ShaderCompiledRef compiledRef = LoadCSO(filePath);
				
				ASSERT(compiledRef != nullptr, "Failed to load cso");

				compiledShaders.push_back(compiledRef);

				return compiledRef;
			} else {
				return *it;
			}
		}

		ShaderVariantRef GetShaderVariant(const ShaderVariantDesc & desc) {
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

	};

	using DX12ShaderLibrary = Netcode::Graphics::DX12::ShaderLibrary;
	using DX12ShaderLibraryRef = std::shared_ptr<DX12ShaderLibrary>;

}
