#include "pch.h"
#include "NetcodeAssetExporter.h"
#include <NetcodeAssetLib/Exporter.h>
#include <NetcodeFoundation/ArrayView.hpp>
#include <NetcodeAssetLib/Model.h>

std::tuple<std::unique_ptr<uint8_t[]>, size_t> NetcodeAssetExporter::Export(const Model & model) {

	Netcode::Asset::Model NetcodeModel;
	
	std::vector<Netcode::Asset::InputElement> NetcodeInputElements;
	std::vector<Netcode::Asset::LODLevel> NetcodeLods;
	std::vector<Netcode::Asset::Mesh> NetcodeMeshes;

	NetcodeMeshes.reserve(model.meshes.size());
	uint32_t numLods = 0;
	uint32_t numIEs = 0;
	for(const auto & mesh : model.meshes) {
		numLods += mesh.lods.size();
		numIEs += mesh.inputLayout.size();
	}

	NetcodeLods.reserve(numLods);
	NetcodeInputElements.reserve(numIEs);

	uint32_t NetcodeElementIdx = 0;
	uint32_t NetcodeLodIdx = 0;

	std::vector<std::unique_ptr<uint8_t[]>> vbuffers;
	std::vector<std::unique_ptr<uint8_t[]>> ibuffers;

	for(const auto & mesh : model.meshes) {

		uint32_t cIEIdx = NetcodeElementIdx;
		uint32_t cLodIdx = NetcodeLodIdx;
		uint32_t iOffset = 0;
		uint32_t vOffset = 0;
		uint32_t vTotalSize = 0;
		uint32_t iTotalSize = 0;

		for(const auto & inputElement : mesh.inputLayout) {
			Netcode::Asset::InputElement NetcodeIE;
			NetcodeIE.format = inputElement.format;
			NetcodeIE.byteOffset = inputElement.byteOffset;
			NetcodeIE.semanticIndex = inputElement.semanticIndex;
			strcpy_s(NetcodeIE.semanticName, inputElement.semanticName.c_str());
			NetcodeIE.semanticName[31] = '\0';
			NetcodeInputElements.emplace_back(NetcodeIE);
			NetcodeElementIdx += 1;
		}

		for(const auto & lod : mesh.lods) {
			Netcode::Asset::LODLevel NetcodeLod;
			NetcodeLod.indexCount = lod.indexCount;
			NetcodeLod.indexBufferByteOffset = iOffset;
			NetcodeLod.indexBufferSizeInBytes = lod.indexDataSizeInBytes;
			NetcodeLod.vertexCount = lod.vertexCount;
			NetcodeLod.vertexBufferByteOffset = vOffset;
			NetcodeLod.vertexBufferSizeInBytes = lod.vertexDataSizeInBytes;
			iOffset += lod.indexDataSizeInBytes;
			vOffset += lod.vertexDataSizeInBytes;
			vTotalSize += lod.vertexDataSizeInBytes;
			iTotalSize += lod.indexDataSizeInBytes;
			NetcodeLods.push_back(NetcodeLod);
			++NetcodeLodIdx;
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

		Netcode::Asset::Mesh NetcodeMesh;
		NetcodeMesh.boundingBox = mesh.boundingBox;
		NetcodeMesh.indices = reinterpret_cast<uint32_t *>(ibuffer.get());
		NetcodeMesh.indicesSizeInBytes = iOffset;
		NetcodeMesh.verticesSizeInBytes = vOffset;
		NetcodeMesh.vertices = reinterpret_cast<void *>(vbuffer.get());
		NetcodeMesh.lodLevels = NetcodeLods.data() + cLodIdx;
		NetcodeMesh.lodLevelsLength = static_cast<uint32_t>(mesh.lods.size());
		NetcodeMesh.vertexSize = mesh.vertexStride;
		NetcodeMesh.inputElements = NetcodeInputElements.data() + cIEIdx;
		NetcodeMesh.inputElementsLength = static_cast<uint32_t>(mesh.inputLayout.size());
		NetcodeMesh.materialId = mesh.materialIdx;

		NetcodeMeshes.push_back(NetcodeMesh);
		vbuffers.emplace_back(std::move(vbuffer));
		ibuffers.emplace_back(std::move(ibuffer));
	}
	
	uint32_t bonesLength = static_cast<uint32_t>(model.skeleton.bones.size());
	std::vector<std::unique_ptr<Netcode::Asset::AnimationKey[]>> keyStorage;
	std::vector<Netcode::Asset::Animation> NetcodeAnims;

	for(const auto & anim : model.animations) {
		Netcode::Asset::Animation NetcodeAnim = {};
		NetcodeAnim.bonesLength = bonesLength;
		NetcodeAnim.keysLength = static_cast<uint32_t>(anim.keyTimes.size());
		NetcodeAnim.ticksPerSecond = anim.framesPerSecond;
		NetcodeAnim.duration = anim.duration;
		NetcodeAnim.times = (float*)anim.keyTimes.data();
		strcpy_s(NetcodeAnim.name, anim.name.c_str());
		NetcodeAnim.name[55] = '\0';

		std::unique_ptr<Netcode::Asset::AnimationKey[]> tempKeys = std::make_unique<Netcode::Asset::AnimationKey[]>(bonesLength * NetcodeAnim.keysLength);
		uint32_t offset = 0;
		for(const auto & boneAnim : anim.keys) {
			memcpy(tempKeys.get() + offset, boneAnim.boneData.data(), sizeof(Netcode::Asset::AnimationKey) * bonesLength);
			offset += bonesLength;
		}

		NetcodeAnim.keys = tempKeys.get();
		keyStorage.emplace_back(std::move(tempKeys));
		NetcodeAnims.push_back(NetcodeAnim);
	}

	std::vector<Netcode::Asset::Collider> NetcodeColliders = model.colliders;

	NetcodeModel.meshes = Netcode::ArrayView<Netcode::Asset::Mesh>(NetcodeMeshes.data(), NetcodeMeshes.size());
	NetcodeModel.animations = Netcode::ArrayView<Netcode::Asset::Animation>(NetcodeAnims.data(), NetcodeAnims.size());
	NetcodeModel.colliders = Netcode::ArrayView<Netcode::Asset::Collider>(NetcodeColliders.data(), NetcodeColliders.size());

	std::vector<Netcode::Asset::Material> NetcodeMats;

	for(const auto & mat : model.materials) {
		Netcode::Asset::Material NetcodeMat = {};
		NetcodeMat.diffuseColor = mat.diffuseColor;
		NetcodeMat.ambientColor = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
		
		strcpy_s(NetcodeMat.diffuseTexture, mat.diffuseMapReference.c_str());
		strcpy_s(NetcodeMat.normalTexture, mat.normalMapReference.c_str());
		strcpy_s(NetcodeMat.ambientTexture, mat.ambientMapReference.c_str());
		strcpy_s(NetcodeMat.roughnessTexture, mat.roughnessMapReference.c_str());
		strcpy_s(NetcodeMat.specularTexture, mat.specularMapReference.c_str());

		NetcodeMat.diffuseTexture[255] = '\0';
		NetcodeMat.normalTexture[255] = '\0';
		NetcodeMat.ambientTexture[255] = '\0';
		NetcodeMat.roughnessTexture[255] = '\0';
		NetcodeMat.specularTexture[255] = '\0';

		NetcodeMats.push_back(NetcodeMat);
	}

	NetcodeModel.materials = Netcode::ArrayView<Netcode::Asset::Material>(NetcodeMats.data(), NetcodeMats.size());
	

	std::vector<Netcode::Asset::Bone> NetcodeBones;

	for(const auto & bone : model.skeleton.bones) {
		Netcode::Asset::Bone NetcodeBone = {};
		strcpy_s(NetcodeBone.name, bone.boneName.c_str());
		NetcodeBone.name[63] = '\0';
		NetcodeBone.parentId = bone.parentIndex;
		NetcodeBone.transform = bone.transform;
		NetcodeBones.push_back(NetcodeBone);
	}

	NetcodeModel.bones = Netcode::ArrayView<Netcode::Asset::Bone>(NetcodeBones.data(), NetcodeBones.size());
	
	size_t s = Netcode::Exporter::CalculateTotalSize(NetcodeModel);
	std::unique_ptr<uint8_t[]> rawData = std::make_unique<uint8_t[]>(s);


	Netcode::Exporter::ExportModelToMemory(rawData.get(), NetcodeModel);

	std::tuple<std::unique_ptr<uint8_t[]>, size_t> rv;
	std::get<0>(rv) = std::move(rawData);
	std::get<1>(rv) = s;

	return rv;
}

