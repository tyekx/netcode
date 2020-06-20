#include "Importer.h"

#include <type_traits>
#include <cstdio>

namespace  Netcode::Asset { 

	namespace Detail {
		template<typename T>
		void ReadArray(T * dst, uint32_t numElements, uint8_t ** data) {
			static_assert(std::is_pod<T>::value, "Must be plain old data type");
			memcpy(dst, *data, sizeof(T) * numElements);
			*data += numElements * sizeof(T);
		}

		template<typename T>
		void Read(T * dst, uint8_t ** data) {
			ReadArray<T>(dst, 1, data);
		}
	}

	void ImportModel(MutableArrayView<uint8_t> buffer, Asset::Model & m) {
		uint32_t meshesSize;
		uint32_t materialsSize;
		uint32_t animDataSize;
		uint32_t colliderDataSize;
		uint32_t boneDataSize;

		uint8_t * it = buffer.begin();

		Detail::Read<uint32_t>(&meshesSize, &it);
		Detail::Read<uint32_t>(&animDataSize, &it);
		Detail::Read<uint32_t>(&materialsSize, &it);
		Detail::Read<uint32_t>(&boneDataSize, &it);
		Detail::Read<uint32_t>(&colliderDataSize, &it);

		void * meshesData = nullptr;
		void * materialsData = nullptr;
		void * animData = nullptr;
		void * colliderData = nullptr;
		void * boneData = nullptr;

		if(meshesSize > 0) {
			meshesData = std::malloc(meshesSize);
			Detail::ReadArray<uint8_t>(static_cast<uint8_t *>(meshesData), meshesSize, &it);
		}

		if(animDataSize > 0) {
			animData = std::malloc(animDataSize);
			Detail::ReadArray<uint8_t>(static_cast<uint8_t *>(animData), animDataSize, &it);
		}

		if(materialsSize > 0) {
			materialsData = std::malloc(materialsSize);
			Detail::ReadArray<uint8_t>(static_cast<uint8_t *>(materialsData), materialsSize, &it);
		}

		if(boneDataSize > 0) {
			boneData = std::malloc(boneDataSize);
			Detail::ReadArray<uint8_t>(static_cast<uint8_t *>(boneData), boneDataSize, &it);
		}

		if(colliderDataSize > 0) {
			colliderData = std::malloc(colliderDataSize);
			Detail::ReadArray<uint8_t>(static_cast<uint8_t *>(colliderData), colliderDataSize, &it);
		}

		m.SetMeshes(meshesData);
		m.SetMaterials(materialsData);
		m.SetAnimData(animData);
		m.SetColliderData(colliderData);
		m.SetBoneData(boneData);
	}
}
