#include "DX12Fence.h"
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {

	void FenceImpl::CreateResources(ID3D12Device * device) {
		DX_API("Failed to create fence")
			device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));

		fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(fenceEvent == NULL) {
			DX_API("Failed to create windows event") HRESULT_FROM_WIN32(GetLastError());
		}
	}

	FenceImpl::FenceImpl(ID3D12Device * device, uint64_t initialValue) : fence{}, fenceEvent{ nullptr }, fenceValue{ initialValue } {
		CreateResources(device);
	}

	ID3D12Fence * FenceImpl::GetFence() const {
		return fence.Get();
	}

	void FenceImpl::Increment() {
		++fenceValue;
	}

	uint64_t FenceImpl::GetValue() const {
		return fenceValue;
	}

	void FenceImpl::HostWait() {
		DX_API("Failed to SetEventOnCompletion")
			fence->SetEventOnCompletion(GetValue(), fenceEvent);

		WaitForSingleObject(fenceEvent, INFINITE);
	}

	void FenceImpl::Signal(ID3D12CommandQueue * commandQueue) {
		Increment();

		DX_API("Failed to signal fence")
			commandQueue->Signal(fence.Get(), GetValue());
	}

	void FenceImpl::Wait(ID3D12CommandQueue * commandQueue) {
		commandQueue->Wait(fence.Get(), GetValue());
	}

}


