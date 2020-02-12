#include "pch.h"
#include "EggAssetExporter.h"
#include <Egg/Exporter.h>

void EggAssetExporter::Export(const std::string & path, const Model & model) {

	Egg::Asset::Model eggModel;
	
	std::vector<Egg::Asset::InputElement> eggInputElements;
	std::vector<Egg::Asset::LODLevel> eggLods;
	std::vector<Egg::Asset::Mesh> eggMeshes;
	eggMeshes.reserve(model.meshes.size());

	uint32_t eggElementIdx = 0;
	uint32_t eggLodIdx = 0;

	std::vector<std::unique_ptr<uint8_t[]>> vbuffers;
	std::vector<std::unique_ptr<uint8_t[]>> ibuffers;

	for(const auto & mesh : model.meshes) {

		uint32_t cIEIdx = eggElementIdx;
		uint32_t cLodIdx = eggLodIdx;
		uint32_t iOffset = 0;
		uint32_t vOffset = 0;
		uint32_t vTotalSize = 0;
		uint32_t iTotalSize = 0;

		for(const auto & inputElement : mesh.inputLayout) {
			Egg::Asset::InputElement eggIE;
			eggIE.format = inputElement.format;
			eggIE.byteOffset = inputElement.byteOffset;
			eggIE.semanticIndex = inputElement.semanticIndex;
			memcpy_s(eggIE.semanticName, 31, inputElement.semanticName.c_str(), inputElement.semanticName.size());
			eggIE.semanticName[31] = '\0';
			eggInputElements.emplace_back(eggIE);
			eggElementIdx += 1;
		}

		for(const auto & lod : mesh.lods) {
			Egg::Asset::LODLevel eggLod;
			eggLod.indexCount = lod.indexCount;
			eggLod.indexBufferByteOffset = iOffset;
			eggLod.indexBufferSizeInBytes = lod.indexDataSizeInBytes;
			eggLod.vertexCount = lod.vertexCount;
			eggLod.vertexBufferByteOffset = vOffset;
			eggLod.vertexBufferSizeInBytes = lod.vertexDataSizeInBytes;
			iOffset += lod.indexDataSizeInBytes;
			vOffset += lod.vertexDataSizeInBytes;
			vTotalSize += lod.vertexDataSizeInBytes;
			iTotalSize += lod.indexDataSizeInBytes;
			eggLods.push_back(eggLod);
			++eggLodIdx;
		}

		std::unique_ptr<uint8_t[]> vbuffer = std::make_unique<uint8_t[]>(vTotalSize);
		std::unique_ptr<uint8_t[]> ibuffer = std::make_unique<uint8_t[]>(iTotalSize);

		iOffset = 0;
		vOffset = 0;

		for(const auto & lod : mesh.lods) {
			memcpy(vbuffer.get() + vOffset, lod.vertexData.get(), lod.vertexDataSizeInBytes);
			memcpy(ibuffer.get() + iOffset, lod.indexData.get(), lod.indexDataSizeInBytes);
			vOffset += lod.vertexDataSizeInBytes;
			iOffset += lod.indexDataSizeInBytes;
		}

		Egg::Asset::Mesh eggMesh;
		eggMesh.boundingBox = mesh.boundingBox;
		eggMesh.indices = reinterpret_cast<uint32_t *>(ibuffer.get());
		eggMesh.indicesSizeInBytes = iOffset;
		eggMesh.verticesSizeInBytes = vOffset;
		eggMesh.vertices = reinterpret_cast<void *>(vbuffer.get());
		eggMesh.lodLevels = eggLods.data() + cLodIdx;
		eggMesh.lodLevelsLength = static_cast<uint32_t>(mesh.lods.size());
		eggMesh.vertexSize = mesh.vertexStride;
		eggMesh.inputElements = eggInputElements.data() + cIEIdx;
		eggMesh.inputElementsLength = static_cast<uint32_t>(mesh.inputLayout.size());
		eggMesh.materialId = mesh.materialIdx;


		eggMeshes.push_back(eggMesh);
		vbuffers.emplace_back(std::move(vbuffer));
		ibuffers.emplace_back(std::move(ibuffer));
	}

	std::vector<Egg::Asset::Animation> eggAnims;

	for(const auto & anim : model.animations) {

	}

	//Egg::Exporter::ExportModel(path.c_str(), ...);

}

