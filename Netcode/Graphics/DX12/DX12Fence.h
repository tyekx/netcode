#pragma once

#include <Netcode/HandleTypes.h>
#include "DX12Common.h"
#include <wrl.h>

namespace Netcode::Graphics::DX12 {

	class FenceImpl : public Netcode::Fence {
		com_ptr<ID3D12Fence1> fence;
		HANDLE fenceEvent;
		uint64_t fenceValue;

		void CreateResources(ID3D12Device * device);
	public:
		FenceImpl(ID3D12Device * device, uint64_t initialValue);

		ID3D12Fence * GetFence() const;

		virtual void Increment() override;

		virtual uint64_t GetValue() const override;

		virtual void HostWait() override;

		void Signal(ID3D12CommandQueue * commandQueue);

		void Wait(ID3D12CommandQueue * commandQueue);

	};

}
