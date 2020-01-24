#include "DX12ResourceViews.h"
#include "DX12Platform.h"
#include "DX12ResourceDesc.h"
#include "DX12Helpers.h"

namespace Egg::Graphics::DX12 {

	ResourceViews::ResourceViews(uint32_t allocationSize, D3D12_GPU_DESCRIPTOR_HANDLE baseGpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE cpu_ShaderVisible, D3D12_CPU_DESCRIPTOR_HANDLE cpu_CpuVisible, D3D12_DESCRIPTOR_HEAP_TYPE heapType, com_ptr<ID3D12Device> device) :
		allocationSize{ allocationSize },
		baseGpuHandle_ShaderVisible{ baseGpuHandle },
		baseCpuHandle_ShaderVisible{ cpu_ShaderVisible },
		baseCpuHandle_CpuVisible{ cpu_CpuVisible },
		heapType{ heapType },
		device{ std::move(device) } {

		/*
		developer asserts
		*/
		if(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV) {
			ASSERT(baseGpuHandle.ptr == 0 && cpu_ShaderVisible.ptr == 0 && cpu_CpuVisible.ptr != 0,
				"When creating a DepthStencilView, shader visibility is not allowed");
		}

		if(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) {
			ASSERT(baseGpuHandle.ptr != 0 && cpu_ShaderVisible.ptr != 0 && cpu_CpuVisible.ptr != 0,
				"When creating a CBV/SRV/UAV, all handles must be valid");
		}
	}

	void ResourceViews::CreateSRV(uint32_t idx, uint64_t resourceHandle) {
		INT offset = static_cast<INT>(idx);

		ASSERT(offset < allocationSize && offset > 0, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "ResourceViews: invalid heap type");

		const GResource & resource = (*reinterpret_cast<const GResource *>(resourceHandle));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvd = GetShaderResourceViewDesc(resource.desc);

		device->CreateShaderResourceView(resource.resource, &srvd, CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_ShaderVisible, offset, Platform::ShaderResourceViewIncrementSize });
	}

	void ResourceViews::CreateRTV(uint32_t idx, uint64_t resourceHandle) {
		INT offset = static_cast<INT>(idx);

		ASSERT(offset < allocationSize && offset > 0, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV, "ResourceViews: invalid heap type");

		const GResource & resource = (*reinterpret_cast<const GResource *>(resourceHandle));

		D3D12_RENDER_TARGET_VIEW_DESC rtvd = GetRenderTargetViewDesc(resource.desc);

		device->CreateRenderTargetView(resource.resource, &rtvd, CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_CpuVisible, offset, Platform::RenderTargetViewIncrementSize });
	}

	void ResourceViews::CreateDSV(uint64_t resourceHandle) {
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV, "ResourceViews: invalid heap type");
		ASSERT(allocationSize == 1, "Array of depth stencil views is not valid");

		const GResource & resource = (*reinterpret_cast<const GResource *>(resourceHandle));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvd = GetDepthStencilViewDesc(resource.desc);

		device->CreateDepthStencilView(resource.resource, &dsvd, baseCpuHandle_CpuVisible);
	}

	void ResourceViews::CreateUAV(uint32_t idx, uint64_t resourceHandle) {
		ASSERT(idx < allocationSize, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "ResourceViews: invalid heap type");

		const GResource & resource = (*reinterpret_cast<const GResource *>(resourceHandle));

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavd = GetUnorderedAccessViewDesc(resource.desc);

		device->CreateUnorderedAccessView(resource.resource, nullptr, &uavd, baseCpuHandle_CpuVisible);
		device->CreateUnorderedAccessView(resource.resource, nullptr, &uavd, baseCpuHandle_ShaderVisible);
	}

	void ResourceViews::CreateSampler(uint32_t idx, uint64_t resourceHandle) {
		ASSERT(idx < allocationSize, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, "ResourceViews: invalid heap type");
		ASSERT(false, "Not implemented");
	}

}
