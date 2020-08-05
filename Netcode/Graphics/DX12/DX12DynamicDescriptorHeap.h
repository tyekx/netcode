#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Decl.h"
#include "DX12Common.h"
#include <wrl.h>

namespace Netcode::Graphics::DX12 {

	class Resource;
	class ResourceViewsImpl;

	class DynamicDescriptorHeap {
		uint32_t srvNumDesc_CpuVisible;
		uint32_t srvNumDesc_ShaderVisible;
		uint32_t dsvNumDesc_CpuVisible;
		uint32_t rtvNumDesc_CpuVisible;
		uint32_t samplerNumDesc_ShaderVisible;

		uint32_t srvOffset_CpuVisible;
		uint32_t srvOffset_ShaderVisible;
		uint32_t dsvOffset_CpuVisible;
		uint32_t rtvOffset_CpuVisible;
		uint32_t samplerOffset_ShaderVisible;

		uint32_t srvNumStatic;
		uint32_t dsvNumStatic;
		uint32_t rtvNumStatic;
		uint32_t samplerNumStatic;

		com_ptr<ID3D12DescriptorHeap> srvDheap_CpuVisible;
		com_ptr<ID3D12DescriptorHeap> srvDheap_ShaderVisible;

		com_ptr<ID3D12DescriptorHeap> dsvDheap_CpuVisible;
		com_ptr<ID3D12DescriptorHeap> rtvDheap_CpuVisible;

		com_ptr<ID3D12DescriptorHeap> samplerDheap_ShaderVisible;

		com_ptr<ID3D12Device> device;

		bool IsRendering;

		DynamicDescriptorHeap() = default;

	public:

		void SetDescriptorHeaps(ID3D12GraphicsCommandList * gcl);

		DynamicDescriptorHeap(com_ptr<ID3D12Device> dev);

		std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> CreateBufferUAV(Ref<DX12::Resource> resource);

		D3D12_GPU_DESCRIPTOR_HANDLE CreateSRV(Ref<DX12::Resource> resource);

		D3D12_GPU_DESCRIPTOR_HANDLE CreateCBV(Ref<DX12::Resource> resource);

		D3D12_CPU_DESCRIPTOR_HANDLE CreateRTV(Ref<DX12::Resource> resource);

		D3D12_CPU_DESCRIPTOR_HANDLE CreateDSV(Ref<DX12::Resource> resource);

		Ref<DX12::ResourceViewsImpl> CreatePermanentSRV(uint32_t numDescriptors);

		Ref<DX12::ResourceViewsImpl> CreatePermanentDSV();

		Ref<DX12::ResourceViewsImpl> CreatePermanentRTV(uint32_t numDescriptors);

		Ref<DX12::ResourceViewsImpl> CreatePermanentSamplers(uint32_t numDescriptors);

		void Prepare();

		// does not deallocate
		void Reset();
	};

}
