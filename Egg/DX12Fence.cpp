#include "DX12Fence.h"

namespace Egg::Graphics::DX12 {

	void Fence::CreateResources(ID3D12Device * device) {
		DX_API("Failed to create fence")
			device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));

		fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(fenceEvent == NULL) {
			DX_API("Failed to create windows event") HRESULT_FROM_WIN32(GetLastError());
		}
	}

	Fence::Fence(ID3D12Device * device, uint64_t initialValue) : fence{}, fenceEvent{ nullptr }, fenceValue{ initialValue } {
		CreateResources(device);
	}

	ID3D12Fence * Fence::GetFence() const {
		return fence.Get();
	}

	void Fence::Increment() {
		++fenceValue;
	}

	uint64_t Fence::GetValue() const {
		return fenceValue;
	}

	void Fence::HostWait() {
		DX_API("Failed to SetEventOnCompletion")
			fence->SetEventOnCompletion(GetValue(), fenceEvent);

		WaitForSingleObject(fenceEvent, INFINITE);
	}

	void Fence::Signal(ID3D12CommandQueue * commandQueue) {
		Increment();

		DX_API("Failed to signal fence")
			commandQueue->Signal(fence.Get(), GetValue());
	}

	void Fence::Wait(ID3D12CommandQueue * commandQueue) {
		commandQueue->Wait(fence.Get(), GetValue());
	}

}


