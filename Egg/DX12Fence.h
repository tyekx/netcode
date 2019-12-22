#pragma once

#include "DX12Common.h"

namespace Egg::Graphics::DX12 {

	class Fence {
		com_ptr<ID3D12Fence1> fence;
		HANDLE fenceEvent;
		UINT64 fenceValue;

	public:
		void CreateResources(ID3D12Device * device) {
			DX_API("Failed to create fence")

				device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
			fenceValue = 0;

			fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if(fenceEvent == NULL) {
				DX_API("Failed to create windows event") HRESULT_FROM_WIN32(GetLastError());
			}
		}

	};

}
