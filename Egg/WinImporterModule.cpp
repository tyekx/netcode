#include "WinImporterModule.h"
#include <DirectXTex/DirectXTex.h>
#include <Windows.h>

void Egg::Module::WinImporterModule::Start(AApp * app)
{
	CoInitialize(NULL);
}

void Egg::Module::WinImporterModule::Shutdown()
{
	CoUninitialize();
}

Egg::HTEXTURE Egg::Module::WinImporterModule::LoadTexture(const std::wstring & imagePath)
{
	return Egg::HTEXTURE();
}

void Egg::Module::WinImporterModule::GenerateMipLevels(Egg::HTEXTURE texture, unsigned short levels)
{
}

Egg::Image * Egg::Module::WinImporterModule::GetImages(Egg::HTEXTURE texture)
{
	return nullptr;
}

unsigned int Egg::Module::WinImporterModule::GetImageCount(Egg::HTEXTURE texture)
{
	return 0;
}

void * Egg::Module::WinImporterModule::LoadFont(const std::wstring & fontPath)
{
	return nullptr;
}
