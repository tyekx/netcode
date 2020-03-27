#include "DX12SpriteFontBuilder.h"

namespace Netcode::Graphics::DX12 {

	SpriteFontBuilder::SpriteFontBuilder(DX12SpriteFontLibraryRef spriteFontLib) : spriteFontLib{ std::move(spriteFontLib) }, spriteFont{ nullptr } {
	
	}

	void SpriteFontBuilder::LoadFont(const std::wstring & mediaPath) {
		spriteFont = spriteFontLib->Get(mediaPath);
	}

	SpriteFontRef SpriteFontBuilder::Build()
	{
		return std::dynamic_pointer_cast<SpriteFont>(spriteFont);
	}

}

