#pragma once

#include "DX12InputLayout.h"
#include "DX12InputLayoutLibrary.h"

namespace Netcode::Graphics::DX12 {

	class InputLayoutBuilder : public Netcode::InputLayoutBuilder {
		BuilderContainer<D3D12_INPUT_ELEMENT_DESC> inputElements;
		Ref<DX12::InputLayoutLibrary> inputLayoutLibrary;
	public:

		InputLayoutBuilder(Memory::ObjectAllocator allocator, Ref<DX12::InputLayoutLibrary> libRef);

		virtual void AddInputElement(const char * semanticName, DXGI_FORMAT format) override;

		virtual void AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format) override;

		virtual void AddInputElement(const char * semanticName, DXGI_FORMAT format, unsigned int byteOffset) override;

		virtual void AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int byteOffset) override;

		virtual Ref<Netcode::InputLayout> Build() override;

	};


}