#pragma once

#include "Model.h"
#include <EggAssetLib/Manifest.h>
#include "EditorApp.h"

using Egg::Module::EditorApp;

class Global {
	~Global() = default;
	Global() = default;
public:

	static std::unique_ptr<Egg::Asset::Manifest> Manifest;
	static std::unique_ptr<Model> Model;
	static std::unique_ptr<EditorApp> EditorApp;


};

