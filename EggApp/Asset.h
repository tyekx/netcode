#pragma once

#include <Egg/Modules.h>
#include <Egg/Asset/Model.h>
#include "Model.h"
#include <map>
#include <string>
#include <Egg/Vertex.h>
#include <Egg/Utility.h>
#include "GameObject.h"
#include "Mesh.h"

using Egg::Graphics::ResourceType;
using Egg::Graphics::ResourceState;

std::map<std::string, std::string> GetPreprocDefs(Egg::Asset::Mesh * mesh, Egg::Asset::Material * material, Egg::Asset::Model * owner) {

	std::map<std::string, std::string> s;

	s["SHADER_CB_USE_PERMESH"];
	s["SHADER_CB_USE_PERFRAME"];
	s["SHADER_CB_USE_PEROBJECT"];
	s["SHADER_NUM_LIGHTS"] = std::to_string(1);

	if(mesh->vertexType == Egg::PNT_Vertex::type ||
	   mesh->vertexType == Egg::PNTWB_Vertex::type ||
	   mesh->vertexType == Egg::PNTTB_Vertex::type ||
	   mesh->vertexType == Egg::PNTWBTB_Vertex::type) {
		s["IAO_HAS_NORMAL"];
		s["IAO_HAS_TEXCOORD"];
	}

	if(mesh->vertexType == Egg::PNTWBTB_Vertex::type ||
	   mesh->vertexType == Egg::PNTWB_Vertex::type) {
		s["IAO_HAS_SKELETON"];
		s["SHADER_CB_USE_BONEDATA"];
	}

	if(mesh->vertexType == Egg::PNTTB_Vertex::type ||
	   mesh->vertexType == Egg::PNTWBTB_Vertex::type) {
		s["IAO_HAS_TANGENT_SPACE"];
	}

	if(material->HasDiffuseTexture()) {
		s["SHADER_TEX_DIFFUSE"];
	}

	if(material->HasNormalTexture()) {
		s["SHADER_TEX_NORMAL"];
	}

	return s;
}

void LoadItem(Egg::Module::IGraphicsModule * g, Egg::Asset::Model * model, Model* modelComponent) {
	for(unsigned int meshIdx = 0; meshIdx < model->meshesLength; ++meshIdx) {
		Egg::Asset::Material * mat = model->materials + model->meshes[meshIdx].materialId;
		Egg::Asset::Mesh * mesh = model->meshes + meshIdx;

		Egg::InputLayoutBuilderRef inputLayoutBuilder = g->CreateInputLayoutBuilder();
		inputLayoutBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0);
		inputLayoutBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 12);
		inputLayoutBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT, 24);

		switch(mesh->vertexType) {
			case Egg::PNTWB_Vertex::type:
			{
				inputLayoutBuilder->AddInputElement("WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT, 32);
				inputLayoutBuilder->AddInputElement("BONEIDS", DXGI_FORMAT_R32_UINT, 44);
			}
			break;
			case Egg::PNTTB_Vertex::type: 
			{
				inputLayoutBuilder->AddInputElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT, 32);
				inputLayoutBuilder->AddInputElement("BINORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 44);
			}
			break;
			case Egg::PNTWBTB_Vertex::type:
			{
				inputLayoutBuilder->AddInputElement("WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT, 32);
				inputLayoutBuilder->AddInputElement("BONEIDS", DXGI_FORMAT_R32_UINT, 44);
				inputLayoutBuilder->AddInputElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT, 48);
				inputLayoutBuilder->AddInputElement("BINORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 60);
			}
			break;
		}

		Egg::InputLayoutRef inputLayout = inputLayoutBuilder->Build();

		Egg::Graphics::UploadBatch batch;

		auto appMesh = std::make_shared<Mesh>();
		
		uint8_t * const vBasePtr = reinterpret_cast<uint8_t *>(mesh->vertices);
		uint8_t * const iBasePtr = reinterpret_cast<uint8_t *>(mesh->indices);
		
		for(unsigned int i = 0; i < mesh->lodLevelsLength; ++i) {
			uint8_t * vData = vBasePtr + mesh->lodLevels[i].vertexOffset * mesh->vertexSize;
			uint8_t * iData = nullptr;
			uint64_t vbuffer = g->resources->CreateVertexBuffer(mesh->lodLevels[i].verticesLength, mesh->vertexSize, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
			uint64_t vCount = mesh->lodLevels[i].vertexCount;
			uint64_t ibuffer = 0;
			uint64_t iCount = 0;

			batch.Upload(vbuffer, vData, mesh->lodLevels[i].verticesLength);
			batch.ResourceBarrier(vbuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);

			if(mesh->indices != nullptr) {
				ibuffer = g->resources->CreateIndexBuffer(mesh->lodLevels[i].indicesLength, DXGI_FORMAT_R32_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
				iData = iBasePtr + 4U * mesh->lodLevels[i].indexOffset;
				iCount = mesh->lodLevels[i].indexCount;

				batch.Upload(ibuffer, iData, iCount * 4U);
				batch.ResourceBarrier(ibuffer, ResourceState::COPY_DEST, ResourceState::INDEX_BUFFER);
			}

			GBuffer lod;
			lod.indexBuffer = ibuffer;
			lod.vertexBuffer = vbuffer;
			lod.indexCount = iCount;
			lod.vertexCount = vCount;

			appMesh->AddLOD(lod);
			appMesh->vertexSize = mesh->vertexSize;
			appMesh->vertexType = mesh->vertexType;
		}

		appMesh->selectedLod = 0;
		appMesh->boundingBox = mesh->boundingBox;

		auto material = std::make_shared<TestMaterial>();

		material->data.diffuseColor = DirectX::XMFLOAT4A{ mat->diffuseColor.x, mat->diffuseColor.y,mat->diffuseColor.z, 1.0f };
		material->data.fresnelR0 = DirectX::XMFLOAT3{ 0.05f, 0.05f, 0.05f };
		material->data.shininess = mat->shininess;

		g->frame->SyncUpload(batch);

		modelComponent->AddShadedMesh(appMesh, material);
	}

}
