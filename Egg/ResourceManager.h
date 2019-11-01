#pragma once

#include "Resource.h"

#include "Vertex.h"
#include "Asset/Model.h"
#include "Multi.h"

/*
Internal includes
*/
#include "ShaderCodeLibrary.h"
#include "RootSignatureLibrary.h"
#include "GeometryLibrary.h"
#include "ResourceAllocator.h"
#include "GPSOLibrary.h"
#include "TextureLibrary.h"

namespace Egg::Graphics {

	class ResourceManager : public Resource::IUploadResource {
		Internal::ShaderCodeLibrary codeLib;
		Internal::RootSignatureLibrary rsLib;
		Internal::GeometryLibrary geomLib;
		Internal::GPSOLibrary gpsoLib;
		Internal::TextureLibrary textureLib;
		Internal::ResourceAllocator resourceAlloc;
	public:

		ResourceManager() : codeLib{}, rsLib{ }, geomLib{ }, gpsoLib{}, textureLib{}, resourceAlloc{} { }

		virtual void CreateResources(ID3D12Device * device) override {
			rsLib.CreateResources(device);
			geomLib.CreateResources(device);
			gpsoLib.CreateResources(device);
			resourceAlloc.CreateResources(device);
		}

		virtual void ReleaseResources() override {

		}

		virtual void UploadResources(ID3D12GraphicsCommandList * copyCommandList) override {
			geomLib.UploadResources(copyCommandList);
		}

		virtual void ReleaseUploadResources() override {
			geomLib.ReleaseUploadResources();
		}

		Material ComposeMaterial(ID3D12RootSignature * rootSig,
								 ID3D12PipelineState * gpso,
								 const Internal::ShaderBindpointCollection & bindpoints,
								 const D3D12_ROOT_SIGNATURE_DESC & rootSigDesc,
								 Internal::ConstantBufferAllocation<PerMeshCb> perMeshAlloc,
								 D3D12_GPU_DESCRIPTOR_HANDLE texturesDheap) {
			Material mat;

			int rootParam = 0;
			for(const auto & bp : bindpoints) {
				if(bp.type == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER) {
					int cbufferTypeId = GetConstantBufferIdFromName(bp.name.c_str());
					ASSERT(cbufferTypeId != -1, "Failed to look up cbuffer by name");

					mat.SetAssoc(cbufferTypeId, rootParam);
					++rootParam;
				}
			}

			for(UINT i = 0; i < rootSigDesc.NumParameters; ++i) {
				if(rootSigDesc.pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
					mat.texturesRootSigSlot = i;
					break;
				}
			}

			mat.perMeshCbAddr = perMeshAlloc.gpuAddr;
			mat.pso = gpso;
			mat.rootSignature = rootSig;
			mat.texturesDescriptor = texturesDheap;

			return mat;
		}



		Graphics::Internal::PreprocessorDefinitions GetDefinitions(Egg::Asset::Mesh * mesh, Egg::Asset::Material * material, Egg::Asset::Model * owner) {
			Graphics::Internal::PreprocessorDefinitions s;

			s.Define("SHADER_CB_USE_PERMESH");
			s.Define("SHADER_CB_USE_PERFRAME");
			s.Define("SHADER_CB_USE_PEROBJECT");

			if(mesh->vertexType == Egg::PNT_Vertex::type ||
			   mesh->vertexType == Egg::PNTWB_Vertex::type ||
			   mesh->vertexType == Egg::PNTTB_Vertex::type ||
			   mesh->vertexType == Egg::PNTWBTB_Vertex::type) {
				s.Define("IAO_HAS_NORMAL");
				s.Define("IAO_HAS_TEXCOORD");
			}

			if(mesh->vertexType == Egg::PNTWBTB_Vertex::type ||
			   mesh->vertexType == Egg::PNTWB_Vertex::type) {
				s.Define("IAO_HAS_SKELETON");
				s.Define("SHADER_CB_USE_BONEDATA");
			}

			if(mesh->vertexType == Egg::PNTTB_Vertex::type ||
			   mesh->vertexType == Egg::PNTWBTB_Vertex::type) {
				s.Define("IAO_HAS_TANGENT_SPACE");
			}

			if(material->HasDiffuseTexture()) {
				s.Define("SHADER_TEX_DIFFUSE");
			}

			if(material->HasNormalTexture()) {
				s.Define("SHADER_TEX_NORMAL");
			}

			return s;
		}

		Multi LoadAssets(Asset::Model * model) {

			Multi m{ model->meshesLength };

			auto alloc = resourceAlloc.AllocateConstantBuffer<PerObjectCb>();

			m.perObjectCb = alloc.data;
			m.perObjectCbAddr = alloc.gpuAddr;

			for(UINT i = 0; i < model->meshesLength; ++i) {
				Internal::PreprocessorDefinitions defs = GetDefinitions(model->meshes + i, model->materials + i, model);

				Internal::ShaderCodeCollection shaderColl = codeLib.GetShaderCodeCollection(defs);
				shaderColl.ReflectBindpoints();

				Internal::RootSignatureDesc rsDesc{ shaderColl.GetBindpoints() };
				ID3D12RootSignature * rs = rsLib.GetRootSignature(rsDesc);

				Geometry * geom = geomLib.GetGeometry(model->meshes + i);

				ID3D12PipelineState * gpso = gpsoLib.GetPipelineState(rs, shaderColl, geom);

				auto perMeshAlloc = resourceAlloc.AllocateConstantBuffer<PerMeshCb>();

				D3D12_GPU_DESCRIPTOR_HANDLE texturesDheapEntry = textureLib.LoadTextures(model->materials + i);

				m.material[i] = ComposeMaterial(rs, gpso, shaderColl.GetBindpoints(), rsDesc.GetDesc(), perMeshAlloc, texturesDheapEntry);
				m.meshes[i] = geom->GetMesh();
				m.meshes[i].perMeshCb = perMeshAlloc.data;
			}

			return m;
		}

	};

}
