#pragma once

#include "Common.h"
#include <sstream>

namespace Egg {
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
}
