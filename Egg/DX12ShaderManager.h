#pragma once

#include "DX12ShaderCompiler.h"
#include "DX12ShaderLibrary.h"
#include "DX12ShaderSourceLibrary.h"

#include <string>
#include "Path.h"

namespace Egg::Graphics::DX12 {


	class ShaderManager {
		ShaderCompiler compiler;
		ShaderSourceLibrary sourceLibrary;
		ShaderLibrary bytecodeLibrary;
		std::map<unsigned int, ShaderVariant> variants;
		unsigned int variantId;

		unsigned int GetNextVariantId() {
			unsigned int id = variantId;
			variantId += 1;
			return id;
		}

	public:
		ShaderManager() : bytecodeLibrary{}, variants{}, variantId{ 0xFFFF0000 } { }

		ID3DBlob * GetShader(HSHADER handle) {
			return bytecodeLibrary.GetCompiledShader(handle);
		}

		HSHADER LoadShader(const std::wstring & shaderPath) {
			ShaderPath shaderp{ shaderPath };

			HSHADER shaderHandle = bytecodeLibrary.GetShader(shaderPath);

			if(shaderHandle == SHADER_NOT_FOUND) {
				com_ptr<ID3DBlob> cso = ShaderBytecode::LoadCSO(shaderp.GetAbsolutePath());

				ShaderBytecode compiled{ shaderPath, std::move(cso) };

				shaderHandle = bytecodeLibrary.Insert(std::move(compiled));
			}

			return shaderHandle;
		}

		HINCOMPLETESHADER CreateVertexShader() {
			unsigned int id = GetNextVariantId();
			variants[id].SetShaderType(EShaderType::VERTEX);
			return HINCOMPLETESHADER{ id };
		}

		HINCOMPLETESHADER CreatePixelShader() {
			unsigned int id = GetNextVariantId();
			variants[id].SetShaderType(EShaderType::PIXEL);
			return HINCOMPLETESHADER{ id };
		}

		HINCOMPLETESHADER CreateGeometryShader() {
			ASSERT(false, "Feature not supported");
			return HINCOMPLETESHADER{ 0xFFFFFFFF };
		}

		HINCOMPLETESHADER CreateDomainShader() {
			ASSERT(false, "Feature not supported");
			return HINCOMPLETESHADER{ 0xFFFFFFFF };
		}

		HINCOMPLETESHADER CreateHullShader() {
			ASSERT(false, "Feature not supported");
			return HINCOMPLETESHADER{ 0xFFFFFFFF };
		}

		void SetShaderEntry(HINCOMPLETESHADER shader, const std::string & entryFunction) {
			variants[shader.handle].SetEntryFunction(entryFunction);
		}

		void SetShaderSource(HINCOMPLETESHADER shader, const std::wstring & shaderPath) {
			variants[shader.handle].SetSourceReference(shaderPath);
			variants[shader.handle].SetShaderSource(sourceLibrary.GetSource(shaderPath));
		}

		void SetShaderMacros(HINCOMPLETESHADER shader, const std::map<std::string, std::string> & macros) {
			variants[shader.handle].SetMacros(macros);
		}

		HSHADER CompileShader(HINCOMPLETESHADER shader) {
			HSHADER shaderHandle = bytecodeLibrary.GetVariant(variants[shader.handle]);

			if(shaderHandle == SHADER_NOT_FOUND) {
				shaderHandle = bytecodeLibrary.Insert(compiler.Compile(variants[shader.handle]));
			}

			return shaderHandle;
		}

	};


}
