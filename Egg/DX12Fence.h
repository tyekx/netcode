#pragma once

#include "DX12Common.h"
#include "HandleTypes.h"

namespace Egg::Graphics::DX12 {

	class Fence : public Egg::Fence {
		com_ptr<ID3D12Fence1> fence;
		HANDLE fenceEvent;
		UINT64 fenceValue;

		void CreateResources(ID3D12Device * device);
	public:
		Fence(ID3D12Device * device, uint64_t initialValue);

		ID3D12Fence * GetFence() const;

		virtual void Increment() override;

		virtual uint64_t GetValue() const override;

		virtual void HostWait() override;

		void Signal(ID3D12CommandQueue * commandQueue);

		void Wait(ID3D12CommandQueue * commandQueue);

	};

	using DX12Fence = Egg::Graphics::DX12::Fence;
	using DX12FenceRef = std::shared_ptr<DX12Fence>;

}
