#pragma once

#include "Common.h"

namespace Egg::Graphics::Internal {

	template<typename T>
	struct ConstantBufferAllocation {
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddr;
		T * data;
	};

	class ResourceAllocator {

	public:


		template<typename CB_T>
		ConstantBufferAllocation<CB_T> AllocateConstantBuffer() {
			ConstantBufferAllocation<CB_T> alloc;


			return alloc;
		}
	};

}
