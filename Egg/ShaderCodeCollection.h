#pragma once

#include "Common.h"

namespace Egg::Graphics::Internal {

	struct ShaderBindpoint {
		std::string name;
		UINT uid;
		UINT space;
		D3D_SHADER_INPUT_TYPE type;
		D3D12_SHADER_VISIBILITY visibility;

		ShaderBindpoint(const ShaderBindpoint &) = default;
		ShaderBindpoint(const std::string & n, UINT id, UINT sp, D3D_SHADER_INPUT_TYPE tp, D3D12_SHADER_VISIBILITY vis) : name{ n }, uid{ id }, space{ sp }, type{ tp }, visibility{ vis } { }

		bool MatchBindpoint(const ShaderBindpoint & bp) const {
			return uid == bp.uid && space == bp.space && type == bp.type;
		}

		bool MatchingName(const ShaderBindpoint & bp) const {
			return name == bp.name;
		}
	};

	using ShaderBindpointCollection = std::vector<ShaderBindpoint>;
	
	struct ShaderCodeCollection {
		ID3DBlob * vertexShader;
		ID3DBlob * pixelShader;
		ShaderBindpointCollection bindPoints;

		ShaderCodeCollection() : vertexShader{ nullptr }, pixelShader{ nullptr }, bindPoints{} { }
		~ShaderCodeCollection() = default;


		void ReflectBindpoints() {
			GetBindpoints(vertexShader, D3D12_SHADER_VISIBILITY_VERTEX);
			GetBindpoints(pixelShader, D3D12_SHADER_VISIBILITY_PIXEL);
		}

		const ShaderBindpointCollection & GetBindpoints() {
			return bindPoints;
		}

	private:
		void GetBindpoints(ID3DBlob * shaderCode, D3D12_SHADER_VISIBILITY visibility) {
			com_ptr<ID3D12ShaderReflection> reflection;

			DX_API("Failed to reflect shader")
				D3DReflect(shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), IID_PPV_ARGS(reflection.GetAddressOf()));

			D3D12_SHADER_INPUT_BIND_DESC bindPoint;
			int i = 0;
			for(HRESULT hr = reflection->GetResourceBindingDesc(i, &bindPoint); SUCCEEDED(hr); ++i, hr = reflection->GetResourceBindingDesc(i, &bindPoint)) {
				ShaderBindpoint sp{ bindPoint.Name, bindPoint.uID, bindPoint.Space, bindPoint.Type, visibility };
				bool found = false;
				for(auto & existingBindpoint : bindPoints) {
					if(existingBindpoint.MatchBindpoint(sp)) {
						/*
						if the bind point is matching, but the names are not, that means the pixel/vertex/geometry/... shader defined the same bindpoint
						with a (probably) different semantics. This is a problem need to be fixed from the shaders
						*/
						ASSERT(existingBindpoint.MatchingName(sp), "Mismatching name for the same bind point");
						found = true;
						if(existingBindpoint.visibility != visibility) {
							// shader visibility is not a "pipeable" value
							existingBindpoint.visibility = D3D12_SHADER_VISIBILITY_ALL;
						}
					}
				}

				if(!found) {
					bindPoints.push_back(sp);
				}
			}
		}
	};

}
