#pragma once

#include "Modules.h"

namespace Egg::Module {

	class WinImporterModule : public IImporterModule {
		// Inherited via IImporterModule
		virtual void Start(AApp * app) override;
		virtual void Shutdown() override;
		virtual Egg::HTEXTURE LoadTexture(const std::wstring & imagePath) override;
		virtual void GenerateMipLevels(Egg::HTEXTURE texture, unsigned short levels) override;
		virtual Egg::Image * GetImages(Egg::HTEXTURE texture) override;
		virtual unsigned int GetImageCount(Egg::HTEXTURE texture) override;
		virtual void * LoadFont(const std::wstring & fontPath) override;
	};

}
