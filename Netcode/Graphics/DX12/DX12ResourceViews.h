#pragma once

#include "../../HandleTypes.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	class ResourceViews : public Netcode::ResourceViews {
		const uint32_t numDescriptors;
		const D3D12_GPU_DESCRIPTOR_HANDLE baseGpuHandle_ShaderVisible;
		const D3D12_CPU_DESCRIPTOR_HANDLE baseCpuHandle_ShaderVisible;
		const D3D12_CPU_DESCRIPTOR_HANDLE baseCpuHandle_CpuVisible;
		const D3D12_DESCRIPTOR_HEAP_TYPE heapType;
		com_ptr<ID3D12Device> device;

		uint32_t GetIncrementSize() const;
	public:
		ResourceViews(uint32_t numDescriptors,
						D3D12_GPU_DESCRIPTOR_HANDLE baseGpuHandle,
						D3D12_CPU_DESCRIPTOR_HANDLE cpu_ShaderVisible,
						D3D12_CPU_DESCRIPTOR_HANDLE cpu_CpuVisible,
						D3D12_DESCRIPTOR_HEAP_TYPE heapType,
						com_ptr<ID3D12Device> device);

		uint32_t GetNumDescriptors() const;

		D3D12_CPU_DESCRIPTOR_HANDLE GetShaderVisibleCpuHandle(uint32_t idx) const;

		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuVisibleCpuHandle(uint32_t idx) const;

		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(uint32_t idx) const;

		void CreateRTV(uint32_t idx, ID3D12Resource * resource, DXGI_FORMAT format);

		virtual ~ResourceViews() = default;

		virtual void CreateSRV(uint32_t idx, GpuResourcePtr resourceHandle) override;

		virtual void CreateSRV(uint32_t idx, GpuResourcePtr resourceHandle, uint32_t firstElement, uint32_t numElements) override;

		virtual void CreateRTV(uint32_t idx, GpuResourcePtr resourceHandle) override;

		virtual void CreateDSV(GpuResourcePtr resourceHandle)  override;

		virtual void CreateUAV(uint32_t idx, GpuResourcePtr resourceHandle) override;

		virtual void CreateSampler(uint32_t idx, GpuResourcePtr resourceHandle)  override;
	};

	using DX12ResourceViews = Netcode::Graphics::DX12::ResourceViews;
	using DX12ResourceViewsRef = std::shared_ptr<DX12ResourceViews>;

}
