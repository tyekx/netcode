#pragma once

#include "../Common.h"
#include "Material.h"
#include "Geometry.h"
#include "../PsoManager.h"
#include <typeinfo>
#include <map>
#include <string>

namespace Egg {
	namespace Mesh {

		GG_CLASS(Shaded)
			com_ptr<ID3D12PipelineState> pipelineState;
			D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsoDesc;
			Material::P material;
			Geometry::P geometry;

			com_ptr<ID3D12RootSignatureDeserializer> rsDeserializer;
			com_ptr<ID3D12ShaderReflection> vsReflection;
			com_ptr<ID3D12ShaderReflection> gsReflection;
			com_ptr<ID3D12ShaderReflection> psReflection;
		public:
			Shaded(PsoManager * psoMan, Material::P mat, Geometry::P geom) : pipelineState{ nullptr }, gpsoDesc{}, material{ mat }, geometry{ geom }, rsDeserializer{ nullptr }, vsReflection{ nullptr }, gsReflection{ nullptr }, psReflection{ nullptr } {
				ZeroMemory(&gpsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
				gpsoDesc.NumRenderTargets = 1;
				gpsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				gpsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
				gpsoDesc.InputLayout = geom->GetInputLayout();
				mat->ApplyToDescriptor(gpsoDesc);
			
				DX_API("Failed to reflect vertex shader")
					D3DReflect(gpsoDesc.VS.pShaderBytecode, gpsoDesc.VS.BytecodeLength, IID_PPV_ARGS(vsReflection.GetAddressOf()));

				DX_API("Failed to reflect pixel shader")
					D3DReflect(gpsoDesc.PS.pShaderBytecode, gpsoDesc.PS.BytecodeLength, IID_PPV_ARGS(psReflection.GetAddressOf()));

				if(gpsoDesc.GS.pShaderBytecode != nullptr) {
					DX_API("Failed to reflect geometry shader")
						D3DReflect(gpsoDesc.GS.pShaderBytecode, gpsoDesc.GS.BytecodeLength, IID_PPV_ARGS(gsReflection.GetAddressOf()));
				}

				DX_API("Failed to deserialize root signature")
					D3D12CreateRootSignatureDeserializer(gpsoDesc.VS.pShaderBytecode, gpsoDesc.VS.BytecodeLength, IID_PPV_ARGS(rsDeserializer.GetAddressOf()));

				pipelineState = psoMan->Get(gpsoDesc);
			}

			template<typename T>
			void BindConstantBuffer(ID3D12GraphicsCommandList * commandList, const T & resource, const std::string & nameOverride = "") {
				std::string name = (nameOverride != "") ? nameOverride.c_str() : typeid(T::Type).name();

				size_t indexOf;
				if((indexOf = name.find(' ')) != std::string::npos) {
					name = name.substr(indexOf + 1);
				}

				D3D12_SHADER_INPUT_BIND_DESC bindDesc;

				ID3D12ShaderReflection* reflections[] = {
					vsReflection.Get(), gsReflection.Get(), psReflection.Get()
				};

				HRESULT hr;
				for(unsigned int i = 0; i < _countof(reflections); ++i) {
					if(!reflections[i]) {
						continue;
					}

					hr = reflections[i]->GetResourceBindingDescByName(name.c_str(), &bindDesc);

					if(SUCCEEDED(hr)) {
						break;
					}
				}

				ASSERT(SUCCEEDED(hr), "Failed to find constant buffer '%s'\r\nPossible errors:\r\n-Optimized away\r\n-Name mismatch\r\n-Wrong shader used", name.c_str());

				const D3D12_ROOT_SIGNATURE_DESC & rootSignatureDesc = *(rsDeserializer->GetRootSignatureDesc());
				for(unsigned int i = 0; i < rootSignatureDesc.NumParameters; ++i) {
					const D3D12_ROOT_PARAMETER & param = rootSignatureDesc.pParameters[i];
					if(param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV &&
					   param.Descriptor.ShaderRegister == bindDesc.BindPoint &&
					   param.Descriptor.RegisterSpace == bindDesc.Space) {
						commandList->SetGraphicsRootConstantBufferView(i, resource.GetGPUVirtualAddress());
						return;
					}
				}

				ASSERT(false, "Failed to bind constant buffer '%s' to root signature.\r\nPossible errors:\r\n-Wrong root signature used on Vertex Shader", name.c_str());
			}

			void SetPipelineState(ID3D12GraphicsCommandList * commandList) {
				commandList->SetPipelineState(pipelineState.Get());
				commandList->SetGraphicsRootSignature(gpsoDesc.pRootSignature);
			}

			void Draw(ID3D12GraphicsCommandList * commandList) {
				geometry->Draw(commandList);
			}

		GG_ENDCLASS


	}
}
