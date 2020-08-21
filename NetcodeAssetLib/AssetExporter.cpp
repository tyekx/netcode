#include "AssetExporter.h"
#include <NetcodeAssetLib/Exporter.h>
#include <NetcodeFoundation/ArrayView.hpp>
#include <NetcodeAssetLib/Model.h>
#include <DirectXTex.h>

namespace Netcode {

	std::tuple<std::unique_ptr<uint8_t[]>, size_t> AssetExporter::Export(const Intermediate::Model & model) {

		Netcode::Asset::Model NetcodeModel;

		std::vector<Netcode::Asset::InputElement> NetcodeInputElements;
		std::vector<Netcode::Asset::LODLevel> NetcodeLods;
		std::vector<Netcode::Asset::Mesh> NetcodeMeshes;
		std::vector<Netcode::Asset::Material> NetcodeMaterials;

		NetcodeMeshes.reserve(model.meshes.size());
		uint32_t numLods = 0;
		uint32_t numIEs = 0;
		for(const auto & mesh : model.meshes) {
			numLods += static_cast<uint32_t>(mesh.lods.size());
			numIEs += static_cast<uint32_t>(mesh.inputLayout.size());
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
			uint32_t vertexStride = 0;

			for(const auto & inputElement : mesh.inputLayout) {
				Netcode::Asset::InputElement NetcodeIE;
				NetcodeIE.format = inputElement.format;
				NetcodeIE.byteOffset = vertexStride;
				NetcodeIE.semanticIndex = inputElement.semanticIndex;
				memset(NetcodeIE.semanticName, 0, sizeof(NetcodeIE.semanticName));
				strcpy_s(NetcodeIE.semanticName, inputElement.semanticName.c_str());
				NetcodeIE.semanticName[31] = '\0';
				NetcodeInputElements.emplace_back(NetcodeIE);
				NetcodeElementIdx += 1;
				vertexStride += static_cast<uint32_t>(DirectX::BitsPerPixel(NetcodeIE.format) / 8);
			}

			for(const auto & lod : mesh.lods) {
				Netcode::Asset::LODLevel NetcodeLod;
				NetcodeLod.indexCount = lod.indexCount;
				NetcodeLod.indexBufferByteOffset = iOffset;
				NetcodeLod.indexBufferSizeInBytes = static_cast<uint32_t>(lod.indexDataSizeInBytes);
				NetcodeLod.vertexCount = lod.vertexCount;
				NetcodeLod.vertexBufferByteOffset = vOffset;
				NetcodeLod.vertexBufferSizeInBytes = lod.vertexCount * vertexStride;
				iOffset += static_cast<uint32_t>(lod.indexDataSizeInBytes);
				vOffset += NetcodeLod.vertexBufferSizeInBytes;
				vTotalSize += NetcodeLod.vertexBufferSizeInBytes;
				iTotalSize += static_cast<uint32_t>(lod.indexDataSizeInBytes);
				NetcodeLods.push_back(NetcodeLod);
				++NetcodeLodIdx;
			}

			std::unique_ptr<uint8_t[]> vbuffer = std::make_unique<uint8_t[]>(vTotalSize);
			std::unique_ptr<uint8_t[]> ibuffer = std::make_unique<uint8_t[]>(iTotalSize);

			iOffset = 0;
			vOffset = 0;

			for(const auto & lod : mesh.lods) {
				memcpy(ibuffer.get() + iOffset, lod.indexData.get(), lod.indexDataSizeInBytes);

				uint32_t iterStride = 0;
				for(const auto & ie : mesh.inputLayout) {
					uint32_t numBytes = static_cast<uint32_t>(DirectX::BitsPerPixel(ie.format) / 8);

					for(uint32_t i = 0; i < lod.vertexCount; ++i) {
						uint8_t * vData = vbuffer.get() + vOffset + i * vertexStride + iterStride;
						memcpy(vData, lod.vertexData.get() + i * mesh.vertexStride + ie.byteOffset, numBytes);
					}

					iterStride += numBytes;
				}

				vOffset += lod.vertexCount * vertexStride;
				iOffset += static_cast<uint32_t>(lod.indexDataSizeInBytes);
			}

			Netcode::Asset::Mesh NetcodeMesh;
			NetcodeMesh.boundingBox = mesh.boundingBox;
			NetcodeMesh.indices = reinterpret_cast<uint32_t *>(ibuffer.get());
			NetcodeMesh.indicesSizeInBytes = iOffset;
			NetcodeMesh.verticesSizeInBytes = vOffset;
			NetcodeMesh.vertices = reinterpret_cast<void *>(vbuffer.get());
			NetcodeMesh.lodLevels = NetcodeLods.data() + cLodIdx;
			NetcodeMesh.lodLevelsLength = static_cast<uint32_t>(mesh.lods.size());
			NetcodeMesh.vertexSize = vertexStride;
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
			NetcodeAnim.times = (float *)anim.keyTimes.data();
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
		std::vector<std::unique_ptr<uint8_t[]>> materialStorage;
		std::vector<std::vector<Netcode::Asset::MaterialParamIndex>> materialIndicies;

		for(const auto & mat : model.materials) {
			Netcode::Asset::Material NetcodeMat = {};
			const uint32_t paramCount = mat->GetParameterCount();

			std::vector<Netcode::Asset::MaterialParamIndex> indices;
			std::unique_ptr<uint8_t[]> dataStorage = std::make_unique<uint8_t[]>(65536);

			uint64_t offset = 0;
			for(uint32_t i = 0; i < paramCount; ++i) {
				Netcode::MaterialParam param = mat->GetParameterByIndex(i);
				Netcode::Asset::MaterialParamIndex pi;
				pi.id = param.id;
				pi.size = param.size;
				pi.offset = static_cast<uint16_t>(offset);

				if(param.id < static_cast<uint32_t>(Netcode::MaterialParamId::SENTINEL_TEXTURE_PATHS_BEGIN)) {
					const void * ptr = mat->GetParameterPointerByIndex(i);

					if(ptr == nullptr) {
						pi.size = 0;
						continue;
					}

					memcpy(dataStorage.get() + offset, ptr, param.size);
					pi.offset = static_cast<uint16_t>(offset);
					offset += param.size;
					indices.push_back(pi);
				} else if(param.id < static_cast<uint32_t>(Netcode::MaterialParamId::SENTINEL_TEXTURE_PATHS_END)) {
					const Netcode::URI::Texture & uri = mat->GetRequiredParameter<Netcode::URI::Texture>(param.id);


					if(uri.Empty()) {
						pi.size = 0;
						continue;
					}

					const std::wstring & v = uri.GetFullPath();

					pi.size = static_cast<uint16_t>(v.size() * sizeof(wchar_t));
					memcpy(dataStorage.get() + offset, v.data(), pi.size);
					offset += static_cast<uint32_t>(pi.size);
					indices.push_back(pi);
				}

				Netcode::OutOfRangeAssertion(offset < std::numeric_limits<uint16_t>::max());
			}

			memset(NetcodeMat.name, 0, sizeof(NetcodeMat.name));
			strcpy_s(NetcodeMat.name, mat->GetName().c_str());
			NetcodeMat.type = static_cast<uint32_t>(mat->GetType());
			NetcodeMat.dataSizeInBytes = offset;
			NetcodeMat.indicesLength = static_cast<uint32_t>(indices.size());
			NetcodeMat.indices = indices.data();
			NetcodeMat.data = dataStorage.get();

			materialStorage.emplace_back(std::move(dataStorage));
			materialIndicies.emplace_back(std::move(indices));

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

}
