#include "DX12SpriteFontBuilder.h"

namespace Netcode::Graphics::DX12 {

	SpriteFontBuilder::SpriteFontBuilder(Ref<DX12::SpriteFontLibrary> spriteFontLib) : spriteFontLib{ std::move(spriteFontLib) }, spriteFont{ nullptr } {
	
	}

	void SpriteFontBuilder::LoadFont(const std::wstring & mediaPath) {
		spriteFont = spriteFontLib->Get(mediaPath);
	}

	Ref<Netcode::SpriteFont> SpriteFontBuilder::Build()
	{
		return spriteFont;
	}

}

