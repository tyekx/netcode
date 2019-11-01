#pragma once

#include "Path.h"
#include "Asset/Model.h"

#include "CommittedTexture2D.h"

namespace Egg::Importer {
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

	void ImportModel(const char* filePath, Asset::Model & model);

	void ImportModel(const MediaPath & mediaFile, Asset::Model & model);

	Graphics::Resource::Committed::Texture2D 
		ImportCommittedTexture2D(ID3D12Device * device, const MediaPath & filePath);


}

