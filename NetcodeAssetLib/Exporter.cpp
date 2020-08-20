#include "Exporter.h"
#include <cstdio>

namespace Netcode {

	namespace Exporter {

		uint32_t CalculateAnimDataSize(const Asset::Model & m) {
			uint32_t acc = 0;

			acc += sizeof(uint32_t);
			acc += static_cast<uint32_t>(m.animations.Size() * sizeof(Asset::Animation));
			for(size_t i = 0; i < m.animations.Size(); ++i) {
				acc += sizeof(Asset::AnimationKey) * m.animations[i].bonesLength * m.animations[i].keysLength;
				acc += sizeof(float) * m.animations[i].keysLength;
			}

			return acc;
		}

		uint32_t CalculateMeshesSize(const Asset::Model & m) {
			uint32_t acc = 0;

			acc += sizeof(uint32_t);
			acc += static_cast<uint32_t>(sizeof(Asset::Mesh) * m.meshes.Size());

			for(uint32_t i = 0; i < m.meshes.Size(); ++i) {
				acc += m.meshes[i].indicesSizeInBytes;
				acc += m.meshes[i].verticesSizeInBytes;
				acc += sizeof(uint32_t);
				acc += m.meshes[i].lodLevelsLength * sizeof(Asset::LODLevel);
				acc += sizeof(uint32_t);
				acc += m.meshes[i].inputElementsLength * sizeof(Asset::InputElement);
			}

			return acc;
		}

		uint32_t CalculateMaterialsSize(const Asset::Model & m) {
			uint32_t acc = 0;
			acc += sizeof(uint32_t);

			acc += m.materials.Size() * sizeof(Asset::Material);

			for(size_t i = 0; i < m.materials.Size(); ++i) {
				acc += m.materials[i].indicesLength * sizeof(Asset::MaterialParamIndex);
				acc += m.materials[i].dataSizeInBytes;
			}

			return acc;
		}

		uint32_t CalculateCollidersSize(const Asset::Model & m) {
			uint32_t acc = 0;
			acc += sizeof(uint32_t);
			acc += static_cast<uint32_t>(sizeof(Asset::Collider) * m.colliders.Size());
			return acc;
		}

		uint32_t CalculateBonesSize(const Asset::Model & m) {
			uint32_t acc = 0;
			acc += sizeof(uint32_t);
			acc += static_cast<uint32_t>(sizeof(Asset::Bone) * m.bones.Size());
			return acc;
		}

		uint32_t CalculateTotalSize(const Asset::Model & m) {
			return 5 * sizeof(uint32_t) + CalculateAnimDataSize(m) + CalculateMaterialsSize(m) + CalculateMeshesSize(m) + CalculateCollidersSize(m) + CalculateBonesSize(m);
		}

		class BinaryWriter {
			uint8_t * data;

		public:
			BinaryWriter(uint8_t * d) : data{ d } { }

			template<typename T>
			void Write(const T & v) {
				memcpy(data, &v, sizeof(T));
				data += sizeof(T);
			}

			template<typename T>
			void Write(const T & v, uint32_t s) {
				memcpy(data, &v, sizeof(T) * s);
				data += sizeof(T) * s;
			}
		};

		void ExportModelToMemory(uint8_t * dst, const Asset::Model & m) {
			uint32_t meshesSize = CalculateMeshesSize(m);
			uint32_t materialsSize = CalculateMaterialsSize(m);
			uint32_t animDataSize = CalculateAnimDataSize(m);
			uint32_t colliderDataSize = CalculateCollidersSize(m);
			uint32_t boneDataSize = CalculateBonesSize(m);

			BinaryWriter bw(dst);

			bw.Write(meshesSize);
			bw.Write(animDataSize);
			bw.Write(materialsSize);
			bw.Write(boneDataSize);
			bw.Write(colliderDataSize);

			uint32_t meshesLength = static_cast<uint32_t>(m.meshes.Size());
			uint32_t materialsLength = static_cast<uint32_t>(m.materials.Size());
			uint32_t animsLength = static_cast<uint32_t>(m.animations.Size());
			uint32_t bonesLength = static_cast<uint32_t>(m.bones.Size());
			uint32_t collidersLength = static_cast<uint32_t>(m.colliders.Size());

			bw.Write(meshesLength);
			bw.Write(*m.meshes.Data(), meshesLength);

			for(uint32_t i = 0; i < meshesLength; ++i) {
				bw.Write(*(reinterpret_cast<uint8_t *>(m.meshes[i].vertices)), m.meshes[i].verticesSizeInBytes);
				bw.Write(*(reinterpret_cast<uint8_t *>(m.meshes[i].indices)), m.meshes[i].indicesSizeInBytes);
				bw.Write(m.meshes[i].lodLevelsLength);
				bw.Write(*(m.meshes[i].lodLevels), m.meshes[i].lodLevelsLength);
				bw.Write(m.meshes[i].inputElementsLength);
				bw.Write(*(m.meshes[i].inputElements), m.meshes[i].inputElementsLength);
			}

			bw.Write(animsLength);
			bw.Write(*m.animations.Data(), animsLength);

			for(uint32_t i = 0; i < animsLength; ++i) {
				bw.Write(*m.animations[i].times, m.animations[i].keysLength);
				bw.Write(*m.animations[i].keys, m.animations[i].keysLength * m.animations[i].bonesLength);
			}

			bw.Write(materialsLength);

			for(uint32_t i = 0; i < materialsLength; ++i) {
				bw.Write(m.materials[i].name);
				bw.Write(m.materials[i].type);
				bw.Write(m.materials[i].indicesLength);
				bw.Write(m.materials[i].dataSizeInBytes);
				bw.Write((Asset::MaterialParamIndex *)nullptr);
				bw.Write((uint8_t *)nullptr);
			}

			for(uint32_t i = 0; i < materialsLength; ++i) {
				bw.Write(*m.materials[i].indices, m.materials[i].indicesLength);
			}

			for(uint32_t i = 0; i < materialsLength; ++i) {
				bw.Write(*m.materials[i].data, m.materials[i].dataSizeInBytes);
			}

			bw.Write(bonesLength);
			bw.Write(*m.bones.Data(), bonesLength);
			bw.Write(collidersLength);
			bw.Write(*m.colliders.Data(), collidersLength);
		}

	}

}

