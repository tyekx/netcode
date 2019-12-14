#pragma once

#include "DX12Common.h"
#include <map>
#include "DX12ShaderVariant.h"
#include <string>
#include <fstream>

namespace Egg::Graphics::DX12 {

	class ShaderBytecode {
	private:
		enum class EType {
			PRECOMPILED = 0, RUNTIME_COMPILED = 1
		};

		EType type;
		com_ptr<ID3DBlob> byteCode;
		ShaderPreprocDefs preprocDefs;
		std::wstring fileReference;
		EShaderType shaderType;

	public:
		ShaderBytecode(const std::wstring & fileRef, com_ptr<ID3DBlob> blob, EShaderType vType = EShaderType::UNDEFINED) :
			type{ EType::PRECOMPILED },
			byteCode{ std::move(blob) },
			preprocDefs{},
			fileReference{ fileRef },
			shaderType{ vType } {
		}

		ShaderBytecode(const std::wstring & fileRef, com_ptr<ID3DBlob> blob, ShaderPreprocDefs defs, EShaderType vType) :
			type{ EType::RUNTIME_COMPILED },
			byteCode{ std::move(blob) },
			preprocDefs{ defs },
			fileReference{ fileRef },
			shaderType{ vType } { 
			ASSERT(shaderType != EShaderType::UNDEFINED, "runtime compiled shader cant have shadertype set to undefined");
		}

		ID3DBlob * GetBlob() const {
			return byteCode.Get();
		}

		static com_ptr<ID3DBlob> LoadCSO(const std::wstring & absolutePath) {
			std::ifstream file{ absolutePath, std::ios::binary | std::ios::ate };

			ASSERT(file.is_open(), "Failed to open blob file: '%S'", absolutePath.c_str());

			std::streamsize size = file.tellg();

			file.seekg(0, std::ios::beg);

			com_ptr<ID3DBlob> shaderByteCode{ nullptr };

			DX_API("Failed to allocate memory for blob")
				D3DCreateBlob((size_t)size, shaderByteCode.GetAddressOf());

			if(file.read(reinterpret_cast<char *>(shaderByteCode->GetBufferPointer()), size)) {
				return shaderByteCode;
			} else {
				throw std::exception{ "Failed to load CSO file" };
			}
		}

		bool operator==(const ShaderBytecode & rhs) const {
			return type == rhs.type && preprocDefs == rhs.preprocDefs && fileReference == rhs.fileReference;
		}

		bool operator==(const ShaderVariant & rhs) const {
			return type == EType::RUNTIME_COMPILED &&
				preprocDefs == rhs.GetPreprocDefs() &&
				fileReference == rhs.GetFileReference() &&
				shaderType == rhs.GetShaderType() &&
				rhs.GetShaderType() != EShaderType::UNDEFINED;
		}
	};

}
