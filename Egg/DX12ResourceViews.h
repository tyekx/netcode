#pragma once

#include "HandleTypes.h"
#include "DX12Common.h"

namespace Egg::Graphics::DX12 {

	class ResourceViews : public Egg::ResourceViews {
		const uint32_t allocationSize;
		const D3D12_GPU_DESCRIPTOR_HANDLE baseGpuHandle_ShaderVisible;
		const D3D12_CPU_DESCRIPTOR_HANDLE baseCpuHandle_ShaderVisible;
		const D3D12_CPU_DESCRIPTOR_HANDLE baseCpuHandle_CpuVisible;
		const D3D12_DESCRIPTOR_HEAP_TYPE heapType;
		com_ptr<ID3D12Device> device;

	public:
		ResourceViews(uint32_t allocationSize,
						D3D12_GPU_DESCRIPTOR_HANDLE baseGpuHandle,
						D3D12_CPU_DESCRIPTOR_HANDLE cpu_ShaderVisible,
						D3D12_CPU_DESCRIPTOR_HANDLE cpu_CpuVisible,
						D3D12_DESCRIPTOR_HEAP_TYPE heapType,
						com_ptr<ID3D12Device> device);

		virtual ~ResourceViews() = default;

		virtual void CreateSRV(uint32_t idx, uint64_t resourceHandle) override;

		virtual void CreateRTV(uint32_t idx, uint64_t resourceHandle) override;

		virtual void CreateDSV(uint64_t resourceHandle)  override;

		virtual void CreateUAV(uint32_t idx, uint64_t resourceHandle) override;

		virtual void CreateSampler(uint32_t idx, uint64_t resourceHandle)  override;
	};

	using DX12ResourceViews = Egg::Graphics::DX12::ResourceViews;
	using DX12ResourceViewsRef = std::shared_ptr<DX12ResourceViews>;

}
