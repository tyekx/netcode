#pragma once

#include "DX12Common.h"
#include "HandleTypes.h"

namespace Egg::Graphics::DX12 {

	class Fence : public Egg::Fence {
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

		virtual void Increment() override {
			++fenceValue;
		}

		virtual uint64_t GetValue() const override {
			return fenceValue;
		}

		virtual void HostWait() override {
			DX_API("Failed to SetEventOnCompletion")
				fence->SetEventOnCompletion(GetValue(), fenceEvent);

			WaitForSingleObject(fenceEvent, INFINITE);
		}

		void Signal(ID3D12CommandQueue * commandQueue) {
			Increment();

			DX_API("Failed to signal fence")
				commandQueue->Signal(fence.Get(), GetValue());
		}

		void Wait(ID3D12CommandQueue * commandQueue) {
			commandQueue->Wait(fence.Get(), GetValue());
		}

	};

	using DX12Fence = Egg::Graphics::DX12::Fence;
	using DX12FenceRef = std::shared_ptr<DX12Fence>;

}
