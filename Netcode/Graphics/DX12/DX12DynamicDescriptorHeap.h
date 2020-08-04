#pragma once

#include "DX12Common.h"
#include "DX12ResourceDesc.h"
#include "DX12ResourceViews.h"
#include "DX12Resource.h"
#include <tuple>

namespace Netcode::Graphics::DX12 {


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

	public:

		void SetDescriptorHeaps(ID3D12GraphicsCommandList * gcl);

		void CreateResources(com_ptr<ID3D12Device> dev);

		std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> CreateBufferUAV(DX12ResourceRef resource);

		D3D12_GPU_DESCRIPTOR_HANDLE CreateSRV(DX12ResourceRef resource);

		D3D12_GPU_DESCRIPTOR_HANDLE CreateCBV(DX12ResourceRef resource);

		D3D12_CPU_DESCRIPTOR_HANDLE CreateRTV(DX12ResourceRef resource);

		D3D12_CPU_DESCRIPTOR_HANDLE CreateDSV(DX12ResourceRef resource);

		Ref<DX12::ResourceViews> CreatePermanentSRV(uint32_t numDescriptors);

		Ref<DX12::ResourceViews> CreatePermanentDSV();

		Ref<DX12::ResourceViews> CreatePermanentRTV(uint32_t numDescriptors);

		Ref<DX12::ResourceViews> CreatePermanentSamplers(uint32_t numDescriptors);

		void Prepare();

		// does not deallocate
		void Reset();
	};

}
