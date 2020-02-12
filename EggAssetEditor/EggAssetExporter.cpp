#include "pch.h"
#include "EggAssetExporter.h"
#include <Egg/Exporter.h>

std::tuple<std::unique_ptr<uint8_t[]>, size_t> EggAssetExporter::Export(const Model & model) {

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
			strcpy_s(eggIE.semanticName, inputElement.semanticName.c_str());
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
	
	uint32_t bonesLength = static_cast<uint32_t>(model.skeleton.bones.size());
	std::vector<std::unique_ptr<Egg::Asset::AnimationKey[]>> keyStorage;
	std::vector<Egg::Asset::Animation> eggAnims;

	for(const auto & anim : model.animations) {
		Egg::Asset::Animation eggAnim = {};
		eggAnim.bonesLength = bonesLength;
		eggAnim.keysLength = static_cast<uint32_t>(anim.keyTimes.size());
		eggAnim.ticksPerSecond = anim.framesPerSecond;
		eggAnim.duration = anim.duration;
		eggAnim.times = (float*)anim.keyTimes.data();
		strcpy_s(eggAnim.name, anim.name.c_str());
		eggAnim.name[55] = '\0';

		std::unique_ptr<Egg::Asset::AnimationKey[]> tempKeys = std::make_unique<Egg::Asset::AnimationKey[]>(bonesLength * eggAnim.keysLength);
		uint32_t offset = 0;
		for(const auto & boneAnim : anim.keys) {
			memcpy(tempKeys.get() + offset, boneAnim.boneData.data(), sizeof(Egg::Asset::AnimationKey) * bonesLength);
			offset += bonesLength;
		}

		eggAnim.keys = tempKeys.get();
		keyStorage.emplace_back(std::move(tempKeys));
		eggAnims.push_back(eggAnim);
	}

	std::vector<Egg::Asset::Collider> eggColliders = model.colliders;

	eggModel.meshes = Egg::ArrayView<Egg::Asset::Mesh>(eggMeshes.data(), eggMeshes.size());
	eggModel.animations = Egg::ArrayView<Egg::Asset::Animation>(eggAnims.data(), eggAnims.size());
	eggModel.colliders = Egg::ArrayView<Egg::Asset::Collider>(eggColliders.data(), eggColliders.size());

	std::vector<Egg::Asset::Material> eggMats;

	for(const auto & mat : model.materials) {
		Egg::Asset::Material eggMat = {};
		eggMat.diffuseColor = mat.diffuseColor;
		eggMat.ambientColor = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
		
		strcpy_s(eggMat.diffuseTexture, mat.diffuseMapReference.c_str());
		strcpy_s(eggMat.normalTexture, mat.normalMapReference.c_str());
		strcpy_s(eggMat.ambientTexture, mat.ambientMapReference.c_str());
		strcpy_s(eggMat.roughnessTexture, mat.roughnessMapReference.c_str());
		strcpy_s(eggMat.specularTexture, mat.specularMapReference.c_str());

		eggMat.diffuseTexture[255] = '\0';
		eggMat.normalTexture[255] = '\0';
		eggMat.ambientTexture[255] = '\0';
		eggMat.roughnessTexture[255] = '\0';
		eggMat.specularTexture[255] = '\0';

		eggMats.push_back(eggMat);
	}

	eggModel.materials = Egg::ArrayView<Egg::Asset::Material>(eggMats.data(), eggMats.size());
	

	std::vector<Egg::Asset::Bone> eggBones;

	for(const auto & bone : model.skeleton.bones) {
		Egg::Asset::Bone eggBone = {};
		strcpy_s(eggBone.name, bone.boneName.c_str());
		eggBone.name[63] = '\0';
		eggBone.parentId = bone.parentIndex;
		eggBone.transform = bone.transform;
		eggBones.push_back(eggBone);
	}

	eggModel.bones = Egg::ArrayView<Egg::Asset::Bone>(eggBones.data(), eggBones.size());
	
	size_t s = Egg::Exporter::CalculateTotalSize(eggModel);
	std::unique_ptr<uint8_t[]> rawData = std::make_unique<uint8_t[]>(s);


	Egg::Exporter::ExportModelToMemory(rawData.get(), eggModel);

	std::tuple<std::unique_ptr<uint8_t[]>, size_t> rv;
	std::get<0>(rv) = std::move(rawData);
	std::get<1>(rv) = s;

	return rv;
}

