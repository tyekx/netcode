#pragma once

#include "Common.h"

namespace Egg::Graphics::Internal {

	template<typename T>
	struct ConstantBufferAllocation {
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddr;
		T * data;

		ConstantBufferAllocation() : gpuAddr{ 0 }, data{ nullptr } { }
	};

	class ResourceAllocator : public Resource::IResource {

		com_ptr<ID3D12Resource> buffer;
		void * mappedPtr;

		UINT64 offset;

	public:
	
		virtual void CreateResources(ID3D12Device * device) override {
			offset = 0;
			UINT64 bufferSize = 8 * 1024 * 1024;

			DX_API("Failed to allocate buffer")
				device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
												D3D12_HEAP_FLAG_NONE,
												&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
												D3D12_RESOURCE_STATE_GENERIC_READ,
												nullptr,
												IID_PPV_ARGS(buffer.GetAddressOf()));

			CD3DX12_RANGE readRange{ 0,0 };

			DX_API("Failed to map buffer")
				buffer->Map(0, &readRange, &mappedPtr);
		}

		virtual void ReleaseResources() override {
			buffer->Unmap(0, nullptr);

			buffer.Reset();
		}

		template<typename CB_T>
		ConstantBufferAllocation<CB_T> AllocateConstantBuffer() {
			ConstantBufferAllocation<CB_T> alloc;

			UINT64 size = sizeof(CB_T);

			alloc.data = reinterpret_cast<CB_T *>(reinterpret_cast<UINT64>(mappedPtr) + offset);
			alloc.gpuAddr = buffer->GetGPUVirtualAddress() + offset;

			offset += size;

			return alloc;
		}
	};

}
