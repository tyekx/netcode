#pragma once

#include "DX12Common.h"
#include <list>
#include <set>
#include <algorithm>
#include <unordered_map>

#include "DX12ResourceDesc.h"
#include "DX12Resource.h"
#include "DX12Heap.h"

namespace Netcode::Graphics::DX12 {

	class HeapManager {

		static size_t GetBucketSize(size_t size);
		
		std::vector<std::shared_ptr<Heap>> heaps;

		com_ptr<ID3D12Device> device;
	public:
		void SetDevice(com_ptr<ID3D12Device> dev);

		Ref<DX12::Resource> CreateResource(const ResourceDesc & d);
	};

}
