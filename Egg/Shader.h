#pragma once

#include "Common.h"
#include <d3dcompiler.h>
#include <map>
#include "Utility.h"
#include "Path.h"

namespace Egg {

	class Shader {

		std::map<std::string, std::string> defines;

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

		std::vector<ShaderBindpoint> bindPoints;
		
		void GetBindpoints(com_ptr<ID3DBlob> shaderCode, D3D12_SHADER_VISIBILITY visibility) {

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

		bool GenerateSamplerState(const ShaderBindpoint & bp, D3D12_STATIC_SAMPLER_DESC & dst) {
			if(bp.name == "linearWrapSampler") {
				ZeroMemory(&dst, sizeof(D3D12_STATIC_SAMPLER_DESC));
				dst.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				dst.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				dst.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				dst.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				dst.RegisterSpace = bp.space;
				dst.ShaderRegister = bp.uid;
				dst.ShaderVisibility = bp.visibility;
				return true;
			}
			return false;
		}

	public:
		void Compile() {
			std::unique_ptr<D3D_SHADER_MACRO[]> preprocDefinitions{ nullptr };

			if(defines.size() > 0) {
				preprocDefinitions = std::make_unique<D3D_SHADER_MACRO[]>(defines.size() + 1);

				int preprocIt = 0;
				for(const auto & kv : defines) {
					preprocDefinitions[preprocIt].Name = kv.first.c_str();
					preprocDefinitions[preprocIt].Definition = (kv.second.size() > 0) ? kv.second.c_str() : nullptr;
					++preprocIt;
				}
				// "NULL" termination, API requires this, dont delete
				preprocDefinitions[preprocIt].Name = nullptr;
				preprocDefinitions[preprocIt].Definition = nullptr;
			}


			/*
			For enabling includes: D3D_COMPILE_STANDARD_FILE_INCLUDE, tho pSourceName must be specified in this case
			*/

			const char * vertexShaderEntry = "Vertex_Main";
			const char * pixelShaderEntry = "Pixel_Main";

			std::string file;
			ShaderPath path{ L"EggShaderLib.hlsli" };
			Egg::Utility::SlurpFile(file, L"C:/work/directx12/Egg/EggShaderLib.hlsli");

			com_ptr<ID3DBlob> vsByteCode;
			com_ptr<ID3DBlob> psByteCode;
			com_ptr<ID3DBlob> errorMsg;

			D3DCompile(file.c_str(), file.size(), nullptr, preprocDefinitions.get(), nullptr, vertexShaderEntry, "vs_5_0", 0, 0, vsByteCode.GetAddressOf(), errorMsg.GetAddressOf());

			if(errorMsg != nullptr) {
				Egg::Utility::Debugf("Error while compiling vertex shader (all warnings are treated as errors): \r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
				return;
			}

			D3DCompile(file.c_str(), file.size(), nullptr, preprocDefinitions.get(), nullptr, pixelShaderEntry, "ps_5_0", 0, 0, psByteCode.GetAddressOf(), errorMsg.GetAddressOf());

			if(errorMsg != nullptr) {
				Egg::Utility::Debugf("Error while compiling pixel shader (all warnings are treated as errors): \r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
				return;
			}

			GetBindpoints(vsByteCode, D3D12_SHADER_VISIBILITY_VERTEX);
			GetBindpoints(psByteCode, D3D12_SHADER_VISIBILITY_PIXEL);

			std::vector<D3D12_ROOT_PARAMETER> rootParams;
			std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;

			// constant buffers first
			for(const auto & i : bindPoints) {
				if(i.type == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER) {
					D3D12_ROOT_PARAMETER cbufferParam;
					cbufferParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
					cbufferParam.Descriptor.RegisterSpace = i.space;
					cbufferParam.Descriptor.ShaderRegister = i.uid;
					cbufferParam.ShaderVisibility = i.visibility;
					
					rootParams.push_back(cbufferParam);
				}
			}

			/* 
			The textures are anticipated in a single descriptor heap
			*/
			int numTextures = 0;
			int minUid = 999999;
			D3D12_SHADER_VISIBILITY visibilty = D3D12_SHADER_VISIBILITY_ALL;
			for(const auto & i : bindPoints) {
				if(i.type == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE) {
					int uidIntVal = static_cast<int>(i.uid);

					if(numTextures == 0) {
						visibilty = i.visibility;
					}

					if(i.visibility != visibilty) {
						visibilty = D3D12_SHADER_VISIBILITY_ALL;
					}

					if(minUid > uidIntVal) {
						minUid = uidIntVal;
					}

					++numTextures;
				}
			}

			D3D12_DESCRIPTOR_RANGE dr;
			dr.BaseShaderRegister = 0;
			dr.NumDescriptors = numTextures;
			dr.RegisterSpace = 0;
			dr.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			dr.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER texturesParam;
			texturesParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			texturesParam.ShaderVisibility = visibilty;
			texturesParam.DescriptorTable.NumDescriptorRanges = 1;
			texturesParam.DescriptorTable.pDescriptorRanges = &dr;
			
			/*
			static samplers last
			*/
			for(const auto & i : bindPoints) {
				if(i.type == D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER) {
					D3D12_STATIC_SAMPLER_DESC sd;
					
					bool generationResult = GenerateSamplerState(i, sd);

					ASSERT(generationResult, "Failed to generate static sampler desc");

					staticSamplers.push_back(sd);
				}
			}

			D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
			rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			rootSigDesc.NumParameters = rootParams.size();
			rootSigDesc.pParameters = &(rootParams.at(0));
			rootSigDesc.NumStaticSamplers = staticSamplers.size();
			rootSigDesc.pStaticSamplers = &(staticSamplers.at(0));
			
			
			com_ptr<ID3DBlob> rootSigBlob;

			
			HRESULT rsResult = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, rootSigBlob.GetAddressOf(), errorMsg.GetAddressOf());

			if(FAILED(rsResult)) {
				Egg::Utility::Debugf("Failed to serialize root signature\r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
			}

			Egg::Utility::Debugf("So far so good2");
		}

		void Define(const std::string & key, float value) {
			Define(key, std::to_string(value));
		}

		void Define(const std::string & key, int value) {
			Define(key, std::to_string(value));
		}

		void Define(const std::string & key, const std::string & value) {
			defines[key] = value;
		}

		void Serialize();
	};

}
