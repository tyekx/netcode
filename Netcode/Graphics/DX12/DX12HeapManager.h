#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Decl.h"
#include "DX12Common.h"
#include <wrl.h>


namespace Netcode::Graphics::DX12 {

	class Heap;
	class Resource;

	class HeapManager {

		static size_t GetBucketSize(size_t size);
		
		std::vector<Ref<Heap>> heaps;

		com_ptr<ID3D12Device> device;
	public:
		HeapManager(com_ptr<ID3D12Device> device);

		Ref<Resource> CreateResource(const ResourceDesc & d);
	};

}
