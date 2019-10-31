#pragma once
#include <Egg/Mesh/Shaded.h>

template<typename T>
struct CbufferAlloc {
	D3D12_GPU_VIRTUAL_ADDRESS addr;
	T * data;

	T * operator->() {
		return data;
	}
};

class ReadOptimizedShadedMesh {
public:
	Egg::Material * material;
	Egg::Mesh::Geometry * geometry;
};

template<typename T, UINT DEQUE_SIZE = 512>
class ReadOptimizedCbVector {
public:


	class Deque {
		T * data;
		com_ptr<ID3D12Resource> resource;
		com_ptr<ID3D12Resource> dhResource;
		D3D12_GPU_VIRTUAL_ADDRESS startAddr;
		UINT8 * mappedPtr;
		int usedSize;
	public:
		Deque() noexcept : data{ nullptr }, resource{ nullptr }, dhResource{ nullptr }, mappedPtr{ nullptr }, usedSize{} { }

		Deque(Deque && o) noexcept : Deque() {
			std::swap(data, o.data);
			std::swap(resource, o.resource);
		}

		Deque & operator=(Deque cp) noexcept {
			std::swap(data, cp.data);
			std::swap(resource, cp.resource);
		}

		~Deque() {
			std::free(data);
			data = nullptr;
			resource.Reset();
		}

		void Upload() {
			memcpy(mappedPtr, data, sizeof(T) * usedSize);
		}

		bool IsFull() const {
			return usedSize == DEQUE_SIZE;
		}

		CbufferAlloc<T> Next() {
			CbufferAlloc<T> alloc;
			alloc.data = reinterpret_cast<T *>(mappedPtr + sizeof(T) * usedSize);
			alloc.addr = startAddr + (usedSize * sizeof(T));
			usedSize += 1;
			return alloc;
		}

		void CopyOnGpu(ID3D12GraphicsCommandList * gcl) {
			gcl->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dhResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
			gcl->CopyBufferRegion(dhResource.Get(), 0, resource.Get(), 0, sizeof(T) * usedSize);
			gcl->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dhResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
		}

		void CreateResources(ID3D12Device * dev) {
			data = (T *)std::malloc(sizeof(T) * DEQUE_SIZE);

			DX_API("Failed to create constant buffer resource")
				dev->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(sizeof(T) * DEQUE_SIZE),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(resource.GetAddressOf()));

			DX_API("Failed to create default hepa cbuffer")
				dev->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(sizeof(T) * DEQUE_SIZE),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(dhResource.GetAddressOf()));

			CD3DX12_RANGE rr(0, 0);
			DX_API("Failed to map constant buffer")
				resource->Map(0, &rr, reinterpret_cast<void **>(&mappedPtr));

			// for debugging purposes, this will name and index the constant buffers for easier identifications
			static int Id = 0;
			std::wstringstream wss;
			wss << "CB(" << typeid(T).name() << ")[" << DEQUE_SIZE << "]#" << Id++;
			resource->SetName(wss.str().c_str());

			startAddr = dhResource->GetGPUVirtualAddress();
		}
	};

	ID3D12Device * device;
	Deque * dequeItems;
	int dequeCount;

	void InsertDeque() {
		new (dequeItems + dequeCount) Deque{};
		dequeItems[dequeCount].CreateResources(device);
		++dequeCount;
	}

	ReadOptimizedCbVector() {
		dequeItems = (Deque *)std::malloc(sizeof(Deque) * 64);
		dequeCount = 0;
	}

	void CreateResources(ID3D12Device * dev) {
		device = dev;

		InsertDeque();
	}

	void CopyOnGpu(ID3D12GraphicsCommandList * gcl) {
		for(int i = 0; i < dequeCount; ++i) {
			dequeItems[i].CopyOnGpu(gcl);
		}
	}

	CbufferAlloc<T> Next() {
		int currentDeque = dequeCount - 1;

		if(dequeItems[currentDeque].IsFull()) {
			InsertDeque();
			currentDeque = dequeCount - 1;
		}

		return dequeItems[currentDeque].Next();
	}

	~ReadOptimizedCbVector()
	{
		for(int i = 0; i < dequeCount; ++i) {
			dequeItems[i].~Deque();
		}
		std::free(dequeItems);
		dequeItems = nullptr;
		dequeCount = 0;
	}

};