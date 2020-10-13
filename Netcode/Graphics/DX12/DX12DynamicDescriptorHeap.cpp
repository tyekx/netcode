#include "DX12DynamicDescriptorHeap.h"
#include <Netcode/Common.h>
#include "DX12Helpers.h"
#include "DX12Platform.h"
#include "DX12Includes.h"
#include "DX12Resource.h"
#include "DX12ResourceViews.h"
#include <tuple>
#include <memory>

namespace Netcode::Graphics::DX12 {

	std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> DynamicDescriptorHeap::CreateBufferUAV(Ptr<DX12::Resource> resource) {
		ASSERT(IsRendering, "Creating transient descriptor outside of rendering is invalid, create them after a Prepare() call");

		const uint32_t srvId_cpuVisible = srvOffset_CpuVisible;
		const uint32_t srvId_shaderVisible = srvOffset_ShaderVisible;
		const D3D12_UNORDERED_ACCESS_VIEW_DESC uavd = GetUnorderedAccessViewDesc(resource->desc, 0);
		const CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle_CpuVisible{ srvDheap_CpuVisible->GetCPUDescriptorHandleForHeapStart(), static_cast<int>(srvId_cpuVisible), Platform::ShaderResourceViewIncrementSize };
		const CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle_ShaderVisible{ srvDheap_ShaderVisible->GetCPUDescriptorHandleForHeapStart(), static_cast<int>(srvId_shaderVisible), Platform::ShaderResourceViewIncrementSize };
		const CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle_ShaderVisible{ srvDheap_ShaderVisible->GetGPUDescriptorHandleForHeapStart(), static_cast<int>(srvId_shaderVisible), Platform::ShaderResourceViewIncrementSize };

		srvOffset_CpuVisible += 1;
		srvOffset_ShaderVisible += 1;

		device->CreateUnorderedAccessView(resource->resource.Get(), nullptr, &uavd, srvCpuHandle_CpuVisible);
		device->CreateUnorderedAccessView(resource->resource.Get(), nullptr, &uavd, srvCpuHandle_ShaderVisible);

		return std::tie(srvCpuHandle_CpuVisible, srvGpuHandle_ShaderVisible);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::CreateRTV(Ptr<DX12::Resource> resource)
	{
		ASSERT(IsRendering, "Creating transient descriptor outside of rendering is invalid, create them after a Prepare() call");

		const uint32_t rtvId_CpuVisible = rtvOffset_CpuVisible;
		const CD3DX12_CPU_DESCRIPTOR_HANDLE dHandle{ rtvDheap_CpuVisible->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(rtvId_CpuVisible), Platform::RenderTargetViewIncrementSize };
		const D3D12_RENDER_TARGET_VIEW_DESC rtvd = GetRenderTargetViewDesc(resource->desc);

		rtvOffset_CpuVisible += 1;

		device->CreateRenderTargetView(resource->resource.Get(), &rtvd, dHandle);

		return dHandle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::CreateDSV(Ptr<DX12::Resource> resource)
	{
		ASSERT(IsRendering, "Creating transient descriptor outside of rendering is invalid, create them after a Prepare() call");
		const uint32_t dsvId_CpuVisible = dsvOffset_CpuVisible;
		const CD3DX12_CPU_DESCRIPTOR_HANDLE dHandle{ dsvDheap_CpuVisible->GetCPUDescriptorHandleForHeapStart(),  static_cast<INT>(dsvId_CpuVisible), Platform::DepthStencilViewIncrementSize };
		const D3D12_DEPTH_STENCIL_VIEW_DESC dsvd = GetDepthStencilViewDesc(resource->desc);

		dsvOffset_CpuVisible += 1;
		
		device->CreateDepthStencilView(resource->resource.Get(), &dsvd, dHandle);

		return dHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::CreateSRV(Ptr<DX12::Resource> resource)
	{
		ASSERT(IsRendering, "Creating transient descriptor outside of rendering is invalid, create them after a Prepare() call");

		const uint32_t srvId_ShaderVisible = srvOffset_ShaderVisible;
		const D3D12_SHADER_RESOURCE_VIEW_DESC srvd = GetShaderResourceViewDesc(resource->desc);
		const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle{ srvDheap_ShaderVisible->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(srvId_ShaderVisible), Platform::ShaderResourceViewIncrementSize };
		const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle{ srvDheap_ShaderVisible->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(srvId_ShaderVisible), Platform::ShaderResourceViewIncrementSize };

		srvOffset_ShaderVisible += 1;

		device->CreateShaderResourceView(resource->resource.Get(), &srvd, cpuHandle);

		return gpuHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::CreateCBV(Ptr<DX12::Resource> resource)
	{
		ASSERT(IsRendering, "Creating transient descriptor outside of rendering is invalid, create them after a Prepare() call");

		const uint32_t srvId_ShaderVisible = srvOffset_ShaderVisible;
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvd;
		cbvd.BufferLocation = resource->resource->GetGPUVirtualAddress();
		cbvd.SizeInBytes = resource->desc.sizeInBytes;
		const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle{ srvDheap_ShaderVisible->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(srvId_ShaderVisible), Platform::ShaderResourceViewIncrementSize };
		const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle{ srvDheap_ShaderVisible->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(srvId_ShaderVisible), Platform::ShaderResourceViewIncrementSize };

		srvOffset_ShaderVisible += 1;

		device->CreateConstantBufferView(&cbvd, cpuHandle);

		return gpuHandle;
	}

	Ref<DX12::ResourceViewsImpl> DynamicDescriptorHeap::CreatePermanentSRV(uint32_t numDescriptors)
	{
		ASSERT(!IsRendering, "Creating permanent descriptor during rendering is invalid, create them before a frame->Prepare() call");

		const uint32_t srvId_cpuVisible = srvOffset_CpuVisible;
		const uint32_t srvId_shaderVisible = srvOffset_ShaderVisible;

		const CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle_CpuVisible{ srvDheap_CpuVisible->GetCPUDescriptorHandleForHeapStart(), static_cast<int>(srvId_cpuVisible), Platform::ShaderResourceViewIncrementSize };
		const CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle_ShaderVisible{ srvDheap_ShaderVisible->GetCPUDescriptorHandleForHeapStart(), static_cast<int>(srvId_shaderVisible), Platform::ShaderResourceViewIncrementSize };
		const CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle_ShaderVisible{ srvDheap_ShaderVisible->GetGPUDescriptorHandleForHeapStart(), static_cast<int>(srvId_shaderVisible), Platform::ShaderResourceViewIncrementSize };

		srvOffset_CpuVisible += numDescriptors;
		srvOffset_ShaderVisible += numDescriptors;
		srvNumStatic += numDescriptors;

		return std::make_shared<DX12::ResourceViewsImpl>(numDescriptors,
													srvGpuHandle_ShaderVisible,
													srvCpuHandle_ShaderVisible,
													srvCpuHandle_CpuVisible,
													D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
													device);
	}

	Ref<DX12::ResourceViewsImpl> DynamicDescriptorHeap::CreatePermanentDSV()
	{
		ASSERT(!IsRendering, "Creating permanent descriptor during rendering is invalid, create them before a frame->Prepare() call");

		const uint32_t dsvId_CpuVisible = dsvOffset_CpuVisible;
		const CD3DX12_CPU_DESCRIPTOR_HANDLE dHandle{ dsvDheap_CpuVisible->GetCPUDescriptorHandleForHeapStart(),  static_cast<INT>(dsvId_CpuVisible), Platform::DepthStencilViewIncrementSize };

		dsvOffset_CpuVisible += 1;
		dsvNumStatic += 1;

		return std::make_shared<DX12::ResourceViewsImpl>(1,
			D3D12_GPU_DESCRIPTOR_HANDLE{},
			D3D12_CPU_DESCRIPTOR_HANDLE{},
			dHandle,
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			device);
	}

	Ref<DX12::ResourceViewsImpl> DynamicDescriptorHeap::CreatePermanentRTV(uint32_t numDescriptors)
	{
		ASSERT(!IsRendering, "Creating permanent descriptor during rendering is invalid, create them before a frame->Prepare() call");
		
		const uint32_t rtvId_CpuVisible = rtvOffset_CpuVisible;
		const CD3DX12_CPU_DESCRIPTOR_HANDLE dHandle{ rtvDheap_CpuVisible->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(rtvId_CpuVisible), Platform::RenderTargetViewIncrementSize };

		rtvOffset_CpuVisible += numDescriptors;
		rtvNumStatic += numDescriptors;

		return std::make_shared<DX12::ResourceViewsImpl>(numDescriptors,
			D3D12_GPU_DESCRIPTOR_HANDLE{},
			D3D12_CPU_DESCRIPTOR_HANDLE{},
			dHandle,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			device);
	}

	Ref<DX12::ResourceViewsImpl> DynamicDescriptorHeap::CreatePermanentSamplers(uint32_t numDescriptors)
	{
		ASSERT(!IsRendering, "Creating permanent descriptor during rendering is invalid, create them before a frame->Prepare() call");
		ASSERT(false, "Samplers are not implemented yet");
		return Ref<DX12::ResourceViewsImpl>{};
	}

	void DynamicDescriptorHeap::Prepare()
	{
		IsRendering = true;

		srvOffset_CpuVisible = srvNumStatic;
		srvOffset_ShaderVisible = srvNumStatic;
		dsvOffset_CpuVisible = dsvNumStatic;
		rtvOffset_CpuVisible = rtvNumStatic;
		samplerOffset_ShaderVisible = samplerNumStatic;
	}

	void DynamicDescriptorHeap::Reset()
	{
		IsRendering = false;

		srvOffset_CpuVisible = srvNumStatic;
		srvOffset_ShaderVisible = srvNumStatic;
		dsvOffset_CpuVisible = dsvNumStatic;
		rtvOffset_CpuVisible = rtvNumStatic;
		samplerOffset_ShaderVisible = samplerNumStatic;
	}

	void DynamicDescriptorHeap::SetDescriptorHeaps(ID3D12GraphicsCommandList * gcl) {
		ID3D12DescriptorHeap * dheaps[2] = {
			srvDheap_ShaderVisible.Get(),
			samplerDheap_ShaderVisible.Get()
		};
		gcl->SetDescriptorHeaps(ARRAYSIZE(dheaps), dheaps);
	}

	DynamicDescriptorHeap::DynamicDescriptorHeap(com_ptr<ID3D12Device> dev) : DynamicDescriptorHeap{}  {
		device = std::move(dev);

		srvNumDesc_CpuVisible = 1 << 16; // 64K
		srvNumDesc_ShaderVisible = 1 << 16; // 64K

		dsvNumDesc_CpuVisible = 1 << 8; // 256
		rtvNumDesc_CpuVisible = 1 << 8; // 256

		samplerNumDesc_ShaderVisible = 1 << 8; // 256

		D3D12_DESCRIPTOR_HEAP_DESC srvDesc_CpuVisible;
		srvDesc_CpuVisible.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		srvDesc_CpuVisible.NodeMask = 0;
		srvDesc_CpuVisible.NumDescriptors = srvNumDesc_CpuVisible;
		srvDesc_CpuVisible.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		D3D12_DESCRIPTOR_HEAP_DESC srvDesc_ShaderVisible;
		srvDesc_ShaderVisible.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvDesc_ShaderVisible.NodeMask = 0;
		srvDesc_ShaderVisible.NumDescriptors = srvNumDesc_ShaderVisible;
		srvDesc_ShaderVisible.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		D3D12_DESCRIPTOR_HEAP_DESC dsvDesc_CpuVisible;
		dsvDesc_CpuVisible.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvDesc_CpuVisible.NodeMask = 0;
		dsvDesc_CpuVisible.NumDescriptors = dsvNumDesc_CpuVisible;
		dsvDesc_CpuVisible.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		D3D12_DESCRIPTOR_HEAP_DESC rtvDesc_CpuVisible;
		rtvDesc_CpuVisible.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvDesc_CpuVisible.NodeMask = 0;
		rtvDesc_CpuVisible.NumDescriptors = rtvNumDesc_CpuVisible;
		rtvDesc_CpuVisible.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		D3D12_DESCRIPTOR_HEAP_DESC samplerDesc_ShaderVisible;
		samplerDesc_ShaderVisible.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		samplerDesc_ShaderVisible.NodeMask = 0;
		samplerDesc_ShaderVisible.NumDescriptors = samplerNumDesc_ShaderVisible;
		samplerDesc_ShaderVisible.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

		DX_API("Failed to create descriptor heap")
			device->CreateDescriptorHeap(&srvDesc_CpuVisible, IID_PPV_ARGS(srvDheap_CpuVisible.ReleaseAndGetAddressOf()));

		DX_API("Failed to create descriptor heap")
			device->CreateDescriptorHeap(&srvDesc_ShaderVisible, IID_PPV_ARGS(srvDheap_ShaderVisible.ReleaseAndGetAddressOf()));

		DX_API("Failed to create descriptor heap")
			device->CreateDescriptorHeap(&dsvDesc_CpuVisible, IID_PPV_ARGS(dsvDheap_CpuVisible.ReleaseAndGetAddressOf()));

		DX_API("Failed to create descriptor heap")
			device->CreateDescriptorHeap(&rtvDesc_CpuVisible, IID_PPV_ARGS(rtvDheap_CpuVisible.ReleaseAndGetAddressOf()));

		DX_API("Failed to create descriptor heap")
			device->CreateDescriptorHeap(&samplerDesc_ShaderVisible, IID_PPV_ARGS(samplerDheap_ShaderVisible.ReleaseAndGetAddressOf()));
	}

}

