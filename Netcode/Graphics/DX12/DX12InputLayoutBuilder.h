#pragma once

#include <Netcode/HandleTypes.h>
#include "DX12InputLayoutLibrary.h"

namespace Netcode::Graphics::DX12 {

	class InputLayoutBuilderImpl : public Netcode::InputLayoutBuilder {
		BuilderContainer<D3D12_INPUT_ELEMENT_DESC> inputElements;
		Ref<InputLayoutLibrary> inputLayoutLibrary;
	public:

		InputLayoutBuilderImpl(Memory::ObjectAllocator allocator, Ref<InputLayoutLibrary> libRef);

		virtual void AddInputElement(const char * semanticName, DXGI_FORMAT format) override;

		virtual void AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format) override;

		virtual void AddInputElement(const char * semanticName, DXGI_FORMAT format, unsigned int byteOffset) override;

		virtual void AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int byteOffset) override;

		virtual Ref<InputLayout> Build() override;

	};


}