#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Common.h"
#include "DX12Decl.h"

namespace Netcode::Graphics::DX12 {

	class StreamOutputImpl;

	class StreamOutputLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<Ref<StreamOutputImpl>> streamOutputs;
	public:

		StreamOutputLibrary(Memory::ObjectAllocator allocator);

		void Insert(Ref<StreamOutputImpl> soRef);

		Ref<StreamOutputImpl> GetStreamOutput(const D3D12_STREAM_OUTPUT_DESC & soDesc);
	};

}
