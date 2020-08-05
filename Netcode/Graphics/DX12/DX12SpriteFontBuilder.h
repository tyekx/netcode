#pragma once

#include <Netcode/HandleTypes.h>
#include <memory>

namespace Netcode::Graphics::DX12 {

	class SpriteFontLibrary;
	class SpriteFontImpl;

	class SpriteFontBuilderImpl : public SpriteFontBuilder {
		Ref<SpriteFontLibrary> spriteFontLib;
		Ref<SpriteFontImpl> spriteFont;
	public:
		SpriteFontBuilderImpl(Ref<SpriteFontLibrary> spriteFontLib);

		virtual void LoadFont(const std::wstring & mediaPath) override;

		virtual Ref<SpriteFont> Build() override;
	};

}
