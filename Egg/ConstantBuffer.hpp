#pragma once

#include "Common.h"
#include <sstream>
#include <memory>
#include <array>

namespace Egg {

	/*
	* T must have a constexpr static int id field. Which should be a unique identifier for that specific Type
	*/
	template<typename T>
	class ConstantBuffer {
		UINT8 * mappedPtr;
		com_ptr<ID3D12Resource> constantBuffer;
		T data;
	public:
		using Type = T;

		ConstantBuffer() : mappedPtr{ nullptr }, constantBuffer{ nullptr } {
			static_assert(__alignof(T) % 16 == 0, "ConstantBuffer type must be aligned to 16 bytes, otherwise you'll get funny glitches, use __declspec(align(16)) before your class specification");
			static_assert(!std::is_polymorphic<T>::value, "Polymorphic classes are strongly discouraged as constant buffers (the Vtable will offset the layout, remove any virtual keywords)");
		}

		~ConstantBuffer() {
			ReleaseResources();
		}

		void CreateResources(ID3D12Device * device) {
			DX_API("Failed to create constant buffer resource")
				device->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(Egg::Utility::Align256(sizeof(T))),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(constantBuffer.GetAddressOf()));

			CD3DX12_RANGE rr(0, 0);
			DX_API("Failed to map constant buffer")
				constantBuffer->Map(0, &rr, reinterpret_cast<void**>(&mappedPtr));

			// for debugging purposes, this will name and index the constant buffers for easier identifications
			static int Id = 0;
			std::wstringstream wss;
			wss << "CB(" << typeid(T).name() << ")#" << Id++;
			constantBuffer->SetName(wss.str().c_str());
		}

		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
			return constantBuffer->GetGPUVirtualAddress();
		}

		void ReleaseResources() {
			if(constantBuffer != nullptr) {
				constantBuffer->Unmap(0, nullptr);
				constantBuffer.Reset();
			}
			mappedPtr = nullptr;
		}

		void Upload() {
			memcpy(mappedPtr, &data, sizeof(T));
		}

		T & operator=(const T & rhs) {
			data = rhs;
			return data;
		}

		T * operator->() {
			return &data;
		}
	};

	template<typename T, unsigned int N_ELEMENTS>
	class ConstantBufferArray {
		UINT8 * mappedPtr;
		com_ptr<ID3D12Resource> constantBuffer;

		__declspec(align(256)) struct PaddedType {
			T value;
		};

		PaddedType data[N_ELEMENTS];
	public:	
		ConstantBufferArray() : mappedPtr{ nullptr }, constantBuffer{ nullptr } {
			static_assert(__alignof(T) % 16 == 0, "ConstantBuffer type must be aligned to 16 bytes, otherwise you'll get funny glitches, use __declspec(align(16)) before your class specification");
			static_assert(!std::is_polymorphic<T>::value, "Polymorphic classes are strongly discouraged as constant buffers (the Vtable will offset the layout, remove any virtual keywords)");
		}

		~ConstantBufferArray() {
			ReleaseResources();
		}

		void CreateResources(ID3D12Device * device) {
			DX_API("Failed to create constant buffer resource")
				device->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(sizeof(PaddedType) * N_ELEMENTS),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(constantBuffer.GetAddressOf()));

			CD3DX12_RANGE rr(0, 0);
			DX_API("Failed to map constant buffer")
				constantBuffer->Map(0, &rr, reinterpret_cast<void **>(&mappedPtr));

			// for debugging purposes, this will name and index the constant buffers for easier identifications
			static int Id = 0;
			std::wstringstream wss;
			wss << "CB(" << typeid(T).name() << ")[" << N_ELEMENTS << "]#" << Id++;
			constantBuffer->SetName(wss.str().c_str());
		}

		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
			return constantBuffer->GetGPUVirtualAddress();
		}

		void ReleaseResources() {
			if(constantBuffer != nullptr) {
				constantBuffer->Unmap(0, nullptr);
				constantBuffer.Reset();
			}
			mappedPtr = nullptr;
		}

		void Upload() {
			memcpy(mappedPtr, data, sizeof(PaddedType) * N_ELEMENTS);
		}

		D3D12_GPU_VIRTUAL_ADDRESS AddressAt(unsigned int i) const {
			D3D12_GPU_VIRTUAL_ADDRESS startAddr = GetGPUVirtualAddress();
			startAddr += sizeof(PaddedType) * i;
			return startAddr;
		}

		T & operator[](unsigned int i) {
			ASSERT(i < N_ELEMENTS, "Out of range");
			return data[i].value;
		}

	};


	/*
	* T must have a constexpr static int id field. Which should be a unique identifier for that specific Type
	* For now this vector can only expand and never shrink.
	*/
	template<typename T, size_t DEQUE_SIZE = 512>
	class ConstantBufferVector {
		ID3D12Device * device;

		class Deque {
			T* data;
			com_ptr<ID3D12Resource> resource;
			UINT8 * mappedPtr;
			int usedSize;
		public:
			Deque() noexcept : data{ nullptr }, resource{ nullptr }, mappedPtr{ nullptr }, usedSize{} { }

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

			T * Next() {
				T * ptr = data + usedSize;
				usedSize += 1;
				return ptr;
			}

			D3D12_GPU_VIRTUAL_ADDRESS AddressAt(int idx) {
				D3D12_GPU_VIRTUAL_ADDRESS startAddr = resource->GetGPUVirtualAddress();
				startAddr += sizeof(T) * idx;
				return startAddr;
			}

			T * operator[](int idx) {
				ASSERT(idx >= 0 && idx < usedSize, "Out of range");
				return data + idx;
			}

			bool IsElement(T *ptr) {
				return data <= ptr && ptr <= data + usedSize;
			}

			D3D12_GPU_VIRTUAL_ADDRESS Translate(T * ptr) {
				UINT64 ptrV = reinterpret_cast<UINT64>(ptr);
				UINT64 startValue = reinterpret_cast<UINT64>(data);
				UINT64 offset = ptrV - startValue;
				UINT64 index = offset / sizeof(T);
				return AddressAt((int)index);
			}

			void CreateResources(ID3D12Device * dev) {
				data = (T*)std::malloc(sizeof(T) * DEQUE_SIZE);

				DX_API("Failed to create constant buffer resource")
					dev->CreateCommittedResource(
						&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
						D3D12_HEAP_FLAG_NONE,
						&CD3DX12_RESOURCE_DESC::Buffer(sizeof(T) * DEQUE_SIZE),
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(resource.GetAddressOf()));

				CD3DX12_RANGE rr(0, 0);
				DX_API("Failed to map constant buffer")
					resource->Map(0, &rr, reinterpret_cast<void **>(&mappedPtr));

				// for debugging purposes, this will name and index the constant buffers for easier identifications
				static int Id = 0;
				std::wstringstream wss;
				wss << "CB(" << typeid(T).name() << ")[" << DEQUE_SIZE << "]#" << Id++;
				resource->SetName(wss.str().c_str());
			}
		};

		std::vector<Deque> dequeVector;
		int elementCount;
	public:
		using Type = T;

		ConstantBufferVector() : device{ nullptr }, dequeVector{}, elementCount{ 0} {
			dequeVector.reserve(32);
			static_assert(__alignof(T) % 256 == 0, "ConstantBuffer type must be aligned to 256 bytes, otherwise you'll get funny glitches, use __declspec(align(256)) before your class specification");
			static_assert(!std::is_polymorphic<T>::value, "Polymorphic classes are strongly discouraged as constant buffers (the Vtable will offset the layout, remove any virtual keywords)");
		}

		void CreateResources(ID3D12Device * dev) {
			device = dev;

			dequeVector.emplace_back();
			dequeVector.at(dequeVector.size() - 1).CreateResources(device);
		}

		void ReleaseResources() {
			dequeVector.clear();
		}

		void Upload() {
			for(int i = 0; i < dequeVector.size(); ++i) {
				dequeVector[i].Upload();
			}
		}

		T * Next() {
			int dequeIdx = elementCount / DEQUE_SIZE;

			if(dequeIdx == dequeVector.size()) {
				ASSERT(dequeIdx < 32, "For now only 32 * DEQUE_SIZE is supported");
				dequeVector.emplace_back();
				dequeVector[dequeIdx].CreateResources(device);
			}
			elementCount++;
			return dequeVector[dequeIdx].Next();
		}

		D3D12_GPU_VIRTUAL_ADDRESS TranslateAddr(T* ptr) {
			for(int i = 0; i < dequeVector.size(); ++i) {
				if(dequeVector[i].IsElement(ptr)) {
					return dequeVector[i].Translate(ptr);
				}
			}
			ASSERT(false, "Addr was not found");
			return 0;
		}

		D3D12_GPU_VIRTUAL_ADDRESS AddressAt(int i) {
			ASSERT(i < elementCount && i >= 0, "Out of range");

			return dequeVector[i / DEQUE_SIZE].AddressAt(i % DEQUE_SIZE);
		}

		T * operator[](int idx) {
			return dequeVector[idx / DEQUE_SIZE][idx % DEQUE_SIZE];
		}

	};



}
