#include "DX12SpriteFontBuilder.h"
#include "DX12SpriteFontLibrary.h"
#include "DX12SpriteFont.h"

namespace Netcode::Graphics::DX12 {

	SpriteFontBuilderImpl::SpriteFontBuilderImpl(Ref<DX12::SpriteFontLibrary> spriteFontLib) : spriteFontLib{ std::move(spriteFontLib) }, spriteFont{ nullptr } {
	
	}

	void SpriteFontBuilderImpl::LoadFont(const std::wstring & mediaPath) {
		spriteFont = spriteFontLib->Get(mediaPath);
	}

	Ref<SpriteFont> SpriteFontBuilderImpl::Build()
	{
		auto tmp = std::move(spriteFont);
		return tmp;
	}

}

