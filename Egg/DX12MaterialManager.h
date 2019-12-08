#pragma once

#include "HandleTypes.h"
#include "DX12GeometryManager.h"
#include "DX12RenderItem.h"
#include <d3dcompiler.h>

namespace Egg::Graphics::DX12 {

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

	class RootSignatureDesc {
		std::vector<D3D12_ROOT_PARAMETER> rootParams;
		std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
		D3D12_DESCRIPTOR_RANGE descRange;
		D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
		std::vector<ShaderBindpoint> cbufferBindpoints;

		bool GenerateSamplerState(const ShaderBindpoint & bp, D3D12_STATIC_SAMPLER_DESC & dst) {
			if(bp.name == "linearWrapSampler") {
				ZeroMemory(&dst, sizeof(D3D12_STATIC_SAMPLER_DESC));
				dst.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				dst.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				dst.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				dst.MaxLOD = 4.0f;
				dst.MinLOD = 0.0f;
				dst.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				dst.RegisterSpace = bp.space;
				dst.ShaderRegister = bp.uid;
				dst.ShaderVisibility = bp.visibility;
				return true;
			}
			return false;
		}

	public:
		bool operator==(const RootSignatureDesc & rs) const {
			if(rootSigDesc.NumParameters != rs.rootSigDesc.NumParameters ||
			   rootSigDesc.NumStaticSamplers != rs.rootSigDesc.NumStaticSamplers ||
			   rootSigDesc.Flags != rs.rootSigDesc.Flags) {
				return false;
			}

			for(UINT i = 0; i < rootSigDesc.NumParameters; ++i) {
				if(memcmp(rootSigDesc.pParameters + i, rs.rootSigDesc.pParameters + i, sizeof(D3D12_ROOT_PARAMETER)) != 0) {
					return false;
				}
			}

			for(UINT i = 0; i < rootSigDesc.NumStaticSamplers; ++i) {
				if(memcmp(rootSigDesc.pStaticSamplers + i, rs.rootSigDesc.pStaticSamplers + i, sizeof(D3D12_STATIC_SAMPLER_DESC)) != 0) {
					return false;
				}
			}

			return true;
		}

		inline bool operator!=(const RootSignatureDesc & rs) const {
			return !operator==(rs);
		}

		RootSignatureDesc() = default;
		RootSignatureDesc(const RootSignatureDesc &) = default;
		RootSignatureDesc & operator=(const RootSignatureDesc &) = default;

		RootSignatureDesc(const std::vector<ShaderBindpoint> & bindPoints) {

			// constant buffers first
			for(const auto & i : bindPoints) {
				if(i.type == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER) {
					D3D12_ROOT_PARAMETER cbufferParam;
					cbufferParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
					cbufferParam.Descriptor.RegisterSpace = i.space;
					cbufferParam.Descriptor.ShaderRegister = i.uid;
					cbufferParam.ShaderVisibility = i.visibility;
					rootParams.push_back(cbufferParam);
					cbufferBindpoints.push_back(i);
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

			descRange.BaseShaderRegister = 0;
			descRange.NumDescriptors = numTextures;
			descRange.RegisterSpace = 0;
			descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER texturesParam;
			texturesParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			texturesParam.ShaderVisibility = visibilty;
			texturesParam.DescriptorTable.NumDescriptorRanges = 1;
			texturesParam.DescriptorTable.pDescriptorRanges = &descRange;

			if(numTextures > 0) {
				rootParams.push_back(texturesParam);
			}

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

			rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			rootSigDesc.NumParameters = static_cast<UINT>(rootParams.size());
			rootSigDesc.pParameters = (rootParams.size() > 0) ? &(rootParams.at(0)) : nullptr;
			rootSigDesc.NumStaticSamplers = static_cast<UINT>(staticSamplers.size());
			rootSigDesc.pStaticSamplers = (staticSamplers.size() > 0) ? &(staticSamplers.at(0)) : nullptr;
		}

		const D3D12_ROOT_SIGNATURE_DESC & GetDesc() const {
			return rootSigDesc;
		}

		UINT GetCbufferBindpoint(const std::string & name) const {
			UINT idx = 0;
			for(const auto & i : cbufferBindpoints) {
				if(i.name == name) {
					return idx;
				}
				++idx;
			}
			return UINT_MAX;
		}
	};

	class MaterialManager {

		struct RootSigItem {
			com_ptr<ID3D12RootSignature> rootSig;
			RootSignatureDesc rootSigDesc;

			UINT GetCbufferBindpoint(const std::string & name) const {
				return rootSigDesc.GetCbufferBindpoint(name);
			}

			ID3D12RootSignature * GetRootSignature() const {
				return rootSig.Get();
			}
		};

		struct PSOItem {
			D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsoDesc;
			UINT rootSigRef;
			com_ptr<ID3D12PipelineState> pipelineState;
		};

		ID3D12Device * device;
		std::vector<RootSigItem> rootSigItems;
		std::vector<PSOItem> items;

		ID3D12PipelineState * GetPSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & gpsoDesc) {
			for(auto & i : items) {
				if(memcmp(&i.gpsoDesc, &gpsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC)) == 0) {
					return i.pipelineState.Get();
				}
			}
			return nullptr;
		}

		PSOItem & GetItem(HMATERIAL mat) {
			return items.at(mat);
		}

		UINT GetRootSigItem(ID3D12RootSignature * ptr) {
			for(size_t i = 0; i < rootSigItems.size(); ++i) {
				if(rootSigItems[i].rootSig.Get() == ptr) {
					return static_cast<UINT>(i);
				}
			}
			return UINT_MAX;
		}

		void ExtractBindpoints(std::vector<ShaderBindpoint> & bindpoints, const void * shaderCode, size_t shaderCodeSize, D3D12_SHADER_VISIBILITY visibility) {
			com_ptr<ID3D12ShaderReflection> reflection;

			DX_API("Failed to reflect shader")
				D3DReflect(shaderCode, shaderCodeSize, IID_PPV_ARGS(reflection.GetAddressOf()));

			D3D12_SHADER_INPUT_BIND_DESC bindPoint;
			int i = 0;
			for(HRESULT hr = reflection->GetResourceBindingDesc(i, &bindPoint); SUCCEEDED(hr); ++i, hr = reflection->GetResourceBindingDesc(i, &bindPoint)) {
				ShaderBindpoint sp{ bindPoint.Name, bindPoint.uID, bindPoint.Space, bindPoint.Type, visibility };
				bool found = false;
				for(auto & existingBindpoint : bindpoints) {
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
					bindpoints.push_back(sp);
				}
			}
		}

		ID3D12RootSignature* CompileRootSignature(const RootSignatureDesc & rd) {
			D3D12_ROOT_SIGNATURE_DESC rootDesc = rd.GetDesc();

			com_ptr<ID3DBlob> rootSigBlob;
			com_ptr<ID3DBlob> errorMsg;

			HRESULT rsResult = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, rootSigBlob.GetAddressOf(), errorMsg.GetAddressOf());

			if(FAILED(rsResult)) {
				Egg::Utility::Debugf("Failed to serialize root signature\r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
			}

			com_ptr<ID3D12RootSignature> rootSignature;

			DX_API("Failed to create root signature")
				device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(rootSignature.GetAddressOf()));

			RootSigItem si;
			si.rootSig = std::move(rootSignature);
			si.rootSigDesc = rd;
			rootSigItems.push_back(si);

			return si.rootSig.Get();
		}
	public:

		void CreateResources(ID3D12Device * dev) {
			device = dev;
		}

		void SetRenderItemMaterial(RenderItem * renderItem, HMATERIAL mat) {
			PSOItem & pso = GetItem(mat);
			renderItem->material = mat;
			renderItem->graphicsPso = pso.pipelineState.Get();
			renderItem->rootSignature = rootSigItems[pso.rootSigRef].GetRootSignature();
		}

		UINT GetCbufferSlot(RenderItem * renderItem, const std::string & name) {
			PSOItem & pso = GetItem(renderItem->material);
			return rootSigItems[pso.rootSigRef].rootSigDesc.GetCbufferBindpoint(name);
		}

		/*
		By contract this is a heavy method to call, forces the user to pre-initialize materials
		*/
		HMATERIAL CreateMaterial(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & gpsoDesc, const D3D12_INPUT_LAYOUT_DESC & inputLayout) {
			static D3D12_SHADER_VISIBILITY shaderVisColl[5] = {
				D3D12_SHADER_VISIBILITY_VERTEX,
				D3D12_SHADER_VISIBILITY_PIXEL,
				D3D12_SHADER_VISIBILITY_GEOMETRY,
				D3D12_SHADER_VISIBILITY_HULL,
				D3D12_SHADER_VISIBILITY_DOMAIN
			};
			D3D12_SHADER_BYTECODE bytecodes[5] = { };
			HMATERIAL handle = -1;

			// the order here is important
			bytecodes[0] = gpsoDesc.VS;
			bytecodes[1] = gpsoDesc.PS;
			bytecodes[2] = gpsoDesc.GS;
			bytecodes[3] = gpsoDesc.HS;
			bytecodes[4] = gpsoDesc.DS;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC copyDesc;
			memcpy(&copyDesc, &gpsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

			copyDesc.InputLayout = inputLayout;

			std::vector<ShaderBindpoint> bindpoints;

			for(int i = 0; i < ARRAYSIZE(bytecodes); ++i) {
				if(bytecodes[i].pShaderBytecode != nullptr) {
					ExtractBindpoints(bindpoints, bytecodes[i].pShaderBytecode, bytecodes[i].BytecodeLength, shaderVisColl[i]);
				}
			}

			RootSignatureDesc rootSigDesc{ bindpoints };
			ID3D12RootSignature * rootSignature = nullptr;

			for(const auto & i : rootSigItems) {
				if(i.rootSigDesc == rootSigDesc) {
					rootSignature = i.rootSig.Get();
				}
			}

			if(rootSignature == nullptr) {
				rootSignature = CompileRootSignature(rootSigDesc);
			}

			ASSERT(rootSignature != nullptr, "Root signature is expected to exist here");
			copyDesc.pRootSignature = rootSignature;

			ID3D12PipelineState * pso = GetPSO(copyDesc);

			if(pso == nullptr) {
				PSOItem si;
				si.gpsoDesc = gpsoDesc;

				DX_API("Failed to create pipeline state")
					device->CreateGraphicsPipelineState(&copyDesc, IID_PPV_ARGS(si.pipelineState.GetAddressOf()));

				si.rootSigRef = GetRootSigItem(rootSignature);

				ASSERT(si.rootSigRef != UINT_MAX, "Root sig reference was not found");

				handle = static_cast<UINT>(items.size());
				pso = si.pipelineState.Get();
				items.push_back(std::move(si));
			}

			ASSERT(pso != nullptr, "Pipeline state is expected to exists here");

			return handle;
		}

	};

}
