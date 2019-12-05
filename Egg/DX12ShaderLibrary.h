#pragma once

#include "DX12ShaderBytecode.h"
#include "HandleTypes.h"

namespace Egg::Graphics::DX12 {

	class ShaderLibrary {

		std::vector<ShaderBytecode> shaders;

	public:

		HSHADER GetShader(const std::wstring & path) {
			return SHADER_NOT_FOUND;
		}

		HSHADER GetVariant(const ShaderVariant & variant) {
			unsigned int id = 0;
			for(const auto & i : shaders) {
				if(i == variant) {
					return id;
				}
				id += 1;
			}
			return SHADER_NOT_FOUND;
		}

		HSHADER Insert(const ShaderBytecode & shader) {
			unsigned int id = static_cast<unsigned int>(shaders.size());

			shaders.push_back(shader);

			return id;
		}

		ID3DBlob * GetCompiledShader(HSHADER shader) {
			return shaders.at(shader).GetBlob();
		}

	};

}
