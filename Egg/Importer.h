#pragma once

#include "Mesh/Geometry.h"
#include "Texture2D.h"
#include "Path.h"
#include "Asset/Model.h"

namespace Egg {
	namespace Importer {
		/* linear one-way allocator for importing / exporting, this does not free memory afterwards, dont use it in a real scenario,
		needs refactor */
		struct LinearAllocator {
			unsigned char * ptr;
			unsigned int nextByte;

			LinearAllocator(unsigned int totalSize) {
				ptr = (unsigned char *)std::malloc(totalSize);
				nextByte = 0;
			}

			void * Allocate(unsigned int n) {
				void * p = ptr + nextByte;
				nextByte += n;
				return p;
			}

			template<typename T>
			T * Allocate() {
				void * p = ptr + nextByte;
				nextByte += sizeof(T);
				return reinterpret_cast<T *>(p);
			}
		};

		Egg::Mesh::Geometry::P ImportSimpleObj(ID3D12Device * device, const MediaPath & filePath);

		Texture2D ImportTexture2D(ID3D12Device * device, const MediaPath & filePath);

		void ImportModel(const char* filePath, Asset::Model & model);

		void ImportModel(const MediaPath & mediaFile, Asset::Model & model);

	};

}

