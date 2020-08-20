#include "DX12ResourceViews.h"
#include <Netcode/Common.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include "DX12Platform.h"
#include "DX12ResourceDesc.h"
#include "DX12Helpers.h"
#include "DX12Resource.h"

namespace Netcode::Graphics::DX12 {

	uint32_t ResourceViewsImpl::GetIncrementSize() const {
		switch(heapType) {
			case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return Platform::ShaderResourceViewIncrementSize;
			case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: return Platform::DepthStencilViewIncrementSize;
			case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: return Platform::RenderTargetViewIncrementSize;
			case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: return Platform::SamplerIncrementSize;
			default: return -1;
		}
	}

	ResourceViewsImpl::ResourceViewsImpl(uint32_t numDescriptors, D3D12_GPU_DESCRIPTOR_HANDLE baseGpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE cpu_ShaderVisible, D3D12_CPU_DESCRIPTOR_HANDLE cpu_CpuVisible, D3D12_DESCRIPTOR_HEAP_TYPE heapType, com_ptr<ID3D12Device> device) :
		numDescriptors{ numDescriptors },
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

	uint32_t ResourceViewsImpl::GetNumDescriptors() const {
		return numDescriptors;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ResourceViewsImpl::GetShaderVisibleCpuHandle(uint32_t idx) const {
		ASSERT(idx < numDescriptors, "Out of range");

		return CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_ShaderVisible, static_cast<INT>(idx), GetIncrementSize() };
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ResourceViewsImpl::GetCpuVisibleCpuHandle(uint32_t idx) const {
		ASSERT(idx < numDescriptors, "Out of range");

		return CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_CpuVisible, static_cast<INT>(idx), GetIncrementSize() };
	}

	D3D12_GPU_DESCRIPTOR_HANDLE ResourceViewsImpl::GetGpuHandle(uint32_t idx) const {
		ASSERT(idx < numDescriptors, "Out of range");

		return CD3DX12_GPU_DESCRIPTOR_HANDLE{ baseGpuHandle_ShaderVisible, static_cast<INT>(idx), GetIncrementSize() };
	}

	void ResourceViewsImpl::CreateRTV(uint32_t idx, ID3D12Resource * resource, DXGI_FORMAT format) {
		INT offset = static_cast<INT>(idx);

		ASSERT(static_cast<uint32_t>(offset) < numDescriptors && offset >= 0, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV, "ResourceViews: invalid heap type");

		D3D12_RENDER_TARGET_VIEW_DESC rtvd;
		rtvd.Texture2D.MipSlice = 0;
		rtvd.Texture2D.PlaneSlice = 0;
		rtvd.Format = format;
		rtvd.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		device->CreateRenderTargetView(resource, &rtvd, CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_CpuVisible, offset, Platform::RenderTargetViewIncrementSize });
	}

	void ResourceViewsImpl::CreateSRV(uint32_t idx, Ptr<GpuResource> resourceHandle) {
		INT offset = static_cast<INT>(idx);

		ASSERT(static_cast<uint32_t>(offset) < numDescriptors && offset >= 0, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "ResourceViews: invalid heap type");

		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle{ baseCpuHandle_ShaderVisible, offset, Platform::ShaderResourceViewIncrementSize };

		Ptr<DX12::Resource> resource = static_cast<Ptr<DX12::Resource>>(resourceHandle);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvd = GetShaderResourceViewDesc(resource->desc);

		device->CreateShaderResourceView(resource->resource.Get(), &srvd, cpuHandle);
	}

	void ResourceViewsImpl::CreateSRV(uint32_t idx, Ptr<GpuResource> resourceHandle, uint32_t firstElement, uint32_t numElements)
	{
		INT offset = static_cast<INT>(idx);

		ASSERT(static_cast<uint32_t>(offset) < numDescriptors && offset >= 0, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "ResourceViews: invalid heap type");

		Ptr<DX12::Resource> resource = static_cast<Ptr<DX12::Resource>>(resourceHandle);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvd = GetShaderResourceViewDesc(resource->desc);

		ASSERT(srvd.ViewDimension == D3D12_SRV_DIMENSION_BUFFER, "ResourceViews: specifing element data only valid for buffers");

		srvd.Buffer.FirstElement = firstElement;
		srvd.Buffer.NumElements = numElements;

		device->CreateShaderResourceView(resource->resource.Get(), &srvd, CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_ShaderVisible, offset, Platform::ShaderResourceViewIncrementSize });
	}

	void ResourceViewsImpl::CreateRTV(uint32_t idx, Ptr<GpuResource> resourceHandle) {
		INT offset = static_cast<INT>(idx);

		ASSERT(static_cast<uint32_t>(offset) < numDescriptors && offset >= 0, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV, "ResourceViews: invalid heap type");

		Ptr<DX12::Resource> resource = static_cast<Ptr<DX12::Resource>>(resourceHandle);

		D3D12_RENDER_TARGET_VIEW_DESC rtvd = GetRenderTargetViewDesc(resource->desc);

		device->CreateRenderTargetView(resource->resource.Get(), &rtvd, CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_CpuVisible, offset, Platform::RenderTargetViewIncrementSize });
	}

	void ResourceViewsImpl::CreateDSV(Ptr<GpuResource> resourceHandle) {
		INT offset = 0;

		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV, "ResourceViews: invalid heap type");
		ASSERT(numDescriptors == 1, "Array of depth stencil views is not valid");

		Ptr<DX12::Resource> resource = static_cast<Ptr<DX12::Resource>>(resourceHandle);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvd = GetDepthStencilViewDesc(resource->desc);

		device->CreateDepthStencilView(resource->resource.Get(), &dsvd, CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_CpuVisible, offset, Platform::DepthStencilViewIncrementSize });
	}

	void ResourceViewsImpl::CreateUAV(uint32_t idx, Ptr<GpuResource> resourceHandle) {
		INT offset = static_cast<INT>(idx);
		ASSERT(idx < numDescriptors, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "ResourceViews: invalid heap type");

		Ptr<DX12::Resource> resource = static_cast<Ptr<DX12::Resource>>(resourceHandle);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavd = GetUnorderedAccessViewDesc(resource->desc);

		device->CreateUnorderedAccessView(resource->resource.Get(), nullptr, &uavd, CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_CpuVisible, offset, Platform::ShaderResourceViewIncrementSize });
		device->CreateUnorderedAccessView(resource->resource.Get(), nullptr, &uavd, CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_ShaderVisible, offset, Platform::ShaderResourceViewIncrementSize });
	}

	void ResourceViewsImpl::CreateSampler(uint32_t idx, Ptr<GpuResource> resourceHandle) {
		ASSERT(idx < numDescriptors, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, "ResourceViews: invalid heap type");
		
		NotImplementedAssertion("");
	}

	void ResourceViewsImpl::ClearSRV(uint32_t idx, Graphics::ResourceDimension expectedResourceDimension)
	{
		INT offset = static_cast<INT>(idx);

		ASSERT(static_cast<uint32_t>(offset) < numDescriptors && offset >= 0, "ResourceViews: idx is out of range");
		ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "ResourceViews: invalid heap type");

		D3D12_SHADER_RESOURCE_VIEW_DESC srvd = {};
		srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch(expectedResourceDimension) {
			case Graphics::ResourceDimension::BUFFER:
				srvd.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				break;
			case Graphics::ResourceDimension::TEXTURE2D:
				srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				break;
			case Graphics::ResourceDimension::TEXTURE3D: 
				srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				break;
			default: UndefinedBehaviourAssertion(false); break;
		}

		device->CreateShaderResourceView(nullptr, &srvd, CD3DX12_CPU_DESCRIPTOR_HANDLE{ baseCpuHandle_ShaderVisible, offset, Platform::ShaderResourceViewIncrementSize });
	}

}
