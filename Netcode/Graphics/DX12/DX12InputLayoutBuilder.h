#pragma once

#include "DX12InputLayout.h"
#include "DX12InputLayoutLibrary.h"

namespace Netcode::Graphics::DX12 {

	class InputLayoutBuilder : public Netcode::InputLayoutBuilder {
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
		DX12InputLayoutLibraryRef inputLayoutLibrary;
	public:

		InputLayoutBuilder(DX12InputLayoutLibraryRef libRef);

		virtual void AddInputElement(const char * semanticName, DXGI_FORMAT format) override;

		virtual void AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format) override;

		virtual void AddInputElement(const char * semanticName, DXGI_FORMAT format, unsigned int byteOffset) override;

		virtual void AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int byteOffset) override;

		virtual InputLayoutRef Build() override;

	};

	using DX12InputLayoutBuilder = Netcode::Graphics::DX12::InputLayoutBuilder;
	using DX12InputLayoutBuilderRef = std::shared_ptr<Netcode::Graphics::DX12::InputLayoutBuilder>;


}