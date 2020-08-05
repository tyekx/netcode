#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Common.h"

namespace Netcode::Graphics {

	class IResourceContext;
	class IFrameContext;

}

namespace Netcode::Graphics::DX12 {

	class SpriteFontImpl;

	class SpriteFontLibrary {

		struct Item {
			Ref<SpriteFontImpl> font;
			std::wstring path;

			Item(Ref<SpriteFontImpl> font, const std::wstring & p);
		};

		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<Item> items;
		Ptr<IResourceContext> resourceCtx;
		Ptr<IFrameContext> frameCtx;

	public:

		SpriteFontLibrary(Memory::ObjectAllocator allocator, Ptr<IResourceContext> resourceCtx, Ptr<IFrameContext> frameCtx);
		Ref<SpriteFontImpl> Get(const std::wstring & mediaPath);

	};

}
