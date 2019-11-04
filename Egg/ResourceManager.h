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
			codeLib.CreateResources(device);
			rsLib.CreateResources(device);
			geomLib.CreateResources(device);
			gpsoLib.CreateResources(device);
			textureLib.CreateResources(device);
			resourceAlloc.CreateResources(device);
		}

		virtual void ReleaseResources() override {

		}

		virtual void UploadResources(ID3D12GraphicsCommandList * copyCommandList) override {
			geomLib.UploadResources(copyCommandList);
			textureLib.UploadResources(copyCommandList);
		}

		virtual void ReleaseUploadResources() override {
			geomLib.ReleaseUploadResources();
			textureLib.ReleaseUploadResources();
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
			s.Define("SHADER_NUM_LIGHTS", 1);

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

		void BeginRender(ID3D12GraphicsCommandList * gcl) {
			textureLib.SetDescriptorHeap(gcl);
		}


		Material FromFiles(const ShaderPath & vs, const ShaderPath & ps, Geometry& geometry) {
			Internal::ShaderCodeCollection coll = codeLib.GetShaderCodeCollection(vs, ps);
			coll.ReflectBindpoints();

			Internal::RootSignatureDesc rsDesc{ coll.GetBindpoints() };
			ID3D12RootSignature * rs = rsLib.GetRootSignature(rsDesc);

			ID3D12PipelineState * gpso = gpsoLib.GetPipelineState(rs, coll, &geometry);

			return ComposeMaterial(rs, gpso, coll.GetBindpoints(), rsDesc.GetDesc(), {}, {});
		}

		Multi LoadAssets(Asset::Model * model) {

			Multi m{ model->meshesLength };

			auto alloc = resourceAlloc.AllocateConstantBuffer<PerObjectCb>();

			m.perObjectCb = alloc.data;
			m.perObjectCbAddr = alloc.gpuAddr;

		    Internal::ConstantBufferAllocation<BoneDataCb> boneDataAlloc; 
			if(model->animationsLength > 0) {
				boneDataAlloc = resourceAlloc.AllocateConstantBuffer<BoneDataCb>();

				m.boneDataCb = boneDataAlloc.data;

				for(int bi = 0; bi < 128; ++bi) {
					DirectX::XMFLOAT4X4A identity;
					DirectX::XMStoreFloat4x4A(&identity, DirectX::XMMatrixIdentity());
					m.boneDataCb->BindTransform[bi] = identity;
					m.boneDataCb->ToRootTransform[bi] = identity;
				}
			}

			for(UINT i = 0; i < model->meshesLength; ++i) {
				Internal::PreprocessorDefinitions defs = GetDefinitions(model->meshes + i, model->materials + i, model);

				Internal::ShaderCodeCollection shaderColl = codeLib.GetShaderCodeCollection(defs);
				shaderColl.ReflectBindpoints();

				Internal::RootSignatureDesc rsDesc{ shaderColl.GetBindpoints() };
				ID3D12RootSignature * rs = rsLib.GetRootSignature(rsDesc);

				Geometry * geom = geomLib.GetGeometry(model->meshes + i);

				ID3D12PipelineState * gpso = gpsoLib.GetPipelineState(rs, shaderColl, geom);

				auto perMeshAlloc = resourceAlloc.AllocateConstantBuffer<PerMeshCb>();
				
				DirectX::XMFLOAT3 kd = model->materials[i].diffuseColor;
				perMeshAlloc.data->diffuseColor = DirectX::XMFLOAT4A{ kd.x, kd.y, kd.z, 1.0f };
				perMeshAlloc.data->fresnelR0 = DirectX::XMFLOAT3{ 0.05f, 0.05f, 0.05f };
				perMeshAlloc.data->shininess = model->materials[i].shininess;

				D3D12_GPU_DESCRIPTOR_HANDLE texturesDheapEntry = textureLib.LoadTextures(model->materials + i);

				m.material[i] = ComposeMaterial(rs, gpso, shaderColl.GetBindpoints(), rsDesc.GetDesc(), perMeshAlloc, texturesDheapEntry);
				m.meshes[i] = geom->GetMesh();
				m.meshes[i].perMeshCb = perMeshAlloc.data;
				if(m.material[i].cbAssoc[BoneDataCb::id] != -1) {
					m.material[i].boneDataCbAddr = boneDataAlloc.gpuAddr;
				}
			}

			return m;
		}

	};

}
