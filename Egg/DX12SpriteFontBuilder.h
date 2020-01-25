#pragma once

#include "HandleTypes.h"
#include "DX12Common.h"
#include "GraphicsContexts.h"
#include "DX12SpriteFontLibrary.h"
#include "DX12Texture.h"

namespace Egg::Graphics::DX12 {

	class SpriteFontBuilder : public Egg::SpriteFontBuilder {
		DX12SpriteFontLibraryRef spriteFontLib;
		DX12SpriteFontRef spriteFont;
	public:
		SpriteFontBuilder(DX12SpriteFontLibraryRef spriteFontLib);

		virtual void LoadFont(const std::wstring & mediaPath) override;

		virtual SpriteFontRef Build() override;
	};

	using DX12SpriteFontBuilder = Egg::Graphics::DX12::SpriteFontBuilder;
	using DX12SpriteFontBuilderRef = std::shared_ptr<DX12SpriteFontBuilder>;

}
