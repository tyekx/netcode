#pragma once

#include "Model.h"
#include <NetcodeAssetLib/Manifest.h>
#include "EditorApp.h"

using Netcode::Module::EditorApp;

class Global {
	~Global() = default;
	Global() = default;
public:

	static std::unique_ptr<Netcode::Asset::Manifest> Manifest;
	static std::unique_ptr<Model> Model;
	static std::unique_ptr<EditorApp> EditorApp;


};

