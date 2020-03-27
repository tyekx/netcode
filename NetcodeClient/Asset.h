#pragma once

#include <Netcode/Modules.h>
#include <map>
#include <string>
#include <Netcode/Vertex.h>
#include <Netcode/Utility.h>
#include "GameObject.h"
#include "Mesh.h"

using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceState;

void LoadItem(Netcode::Module::IGraphicsModule * g, Netcode::Asset::Model * model, Model* modelComponent) {
	for(size_t meshIdx = 0; meshIdx < model->meshes.Size(); ++meshIdx) {
		Netcode::Asset::Material * mat = model->materials.Data() + model->meshes[meshIdx].materialId;
		Netcode::Asset::Mesh * mesh = model->meshes.Data() + meshIdx;

		Netcode::InputLayoutBuilderRef inputLayoutBuilder = g->CreateInputLayoutBuilder();

		for(uint32_t ilIdx = 0; ilIdx < mesh->inputElementsLength; ++ilIdx) {
			inputLayoutBuilder->AddInputElement(mesh->inputElements[ilIdx].semanticName,
												mesh->inputElements[ilIdx].semanticIndex,
												mesh->inputElements[ilIdx].format,
												mesh->inputElements[ilIdx].byteOffset);
		}

		Netcode::InputLayoutRef inputLayout = inputLayoutBuilder->Build();

		Netcode::Graphics::UploadBatch batch;

		auto appMesh = std::make_shared<Mesh>();
		
		uint8_t * const vBasePtr = reinterpret_cast<uint8_t *>(mesh->vertices);
		uint8_t * const iBasePtr = reinterpret_cast<uint8_t *>(mesh->indices);
		
		for(unsigned int i = 0; i < mesh->lodLevelsLength; ++i) {
			uint8_t * vData = vBasePtr + mesh->lodLevels[i].vertexBufferByteOffset;
			uint8_t * iData = nullptr;
			uint64_t vbuffer = g->resources->CreateVertexBuffer(mesh->lodLevels[i].vertexBufferSizeInBytes, mesh->vertexSize, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
			uint64_t vCount = mesh->lodLevels[i].vertexCount;
			uint64_t ibuffer = 0;
			uint64_t iCount = 0;

			batch.Upload(vbuffer, vData, mesh->lodLevels[i].vertexBufferSizeInBytes);
			batch.ResourceBarrier(vbuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);

			if(mesh->indices != nullptr) {
				ibuffer = g->resources->CreateIndexBuffer(mesh->lodLevels[i].indexBufferSizeInBytes, DXGI_FORMAT_R32_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
				iData = iBasePtr + mesh->lodLevels[i].indexBufferByteOffset;
				iCount = mesh->lodLevels[i].indexCount;

				batch.Upload(ibuffer, iData, mesh->lodLevels[i].indexBufferSizeInBytes);
				batch.ResourceBarrier(ibuffer, ResourceState::COPY_DEST, ResourceState::INDEX_BUFFER);
			}

			GBuffer lod;
			lod.indexBuffer = ibuffer;
			lod.vertexBuffer = vbuffer;
			lod.indexCount = iCount;
			lod.vertexCount = vCount;

			appMesh->AddLOD(lod);
			appMesh->vertexSize = mesh->vertexSize;
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
