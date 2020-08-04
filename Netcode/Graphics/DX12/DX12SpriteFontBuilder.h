#pragma once

#include "../../HandleTypes.h"
#include "../GraphicsContexts.h"
#include "DX12Common.h"
#include "DX12SpriteFontLibrary.h"
#include "DX12Texture.h"

namespace Netcode::Graphics::DX12 {

	class SpriteFontBuilder : public Netcode::SpriteFontBuilder {
		Ref<DX12::SpriteFontLibrary> spriteFontLib;
		Ref<DX12::SpriteFont> spriteFont;
	public:
		SpriteFontBuilder(Ref<DX12::SpriteFontLibrary> spriteFontLib);

		virtual void LoadFont(const std::wstring & mediaPath) override;

		virtual Ref<Netcode::SpriteFont> Build() override;
	};

}
