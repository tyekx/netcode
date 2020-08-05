#include "DX12SpriteFontLibrary.h"
#include "DX12SpriteFont.h"

namespace Netcode::Graphics::DX12 {

	SpriteFontLibrary::Item::Item(Ref<SpriteFontImpl> font, const std::wstring & p) : font{ std::move(font) }, path{ p } {

	}

	SpriteFontLibrary::SpriteFontLibrary(Memory::ObjectAllocator allocator, Ptr<IResourceContext> resourceCtx, Ptr<IFrameContext> frameCtx) :
		objectAllocator{ allocator }, items{ BuilderAllocator<Item>{ allocator } }, resourceCtx{ resourceCtx }, frameCtx{ frameCtx } {
	}

	Ref<SpriteFontImpl> SpriteFontLibrary::Get(const std::wstring & mediaPath) {
		auto it = std::find_if(std::begin(items), std::end(items), [&mediaPath](const Item & item) -> bool {
			return item.path == mediaPath;
		});

		if(it == std::end(items)) {

			auto spriteFont = objectAllocator.MakeShared<SpriteFontImpl>(resourceCtx, frameCtx, mediaPath);

			items.emplace_back(Item{ spriteFont, mediaPath });

			return spriteFont;
		}

		return it->font;
	}

}
