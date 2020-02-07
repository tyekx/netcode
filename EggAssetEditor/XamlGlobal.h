#pragma once

#include "Model.h"
#include "Manifest.h"
#include "EditorApp.h"

using Egg::Module::EditorApp;

class Global {
	~Global() = default;
	Global() = default;
public:

	static std::unique_ptr<Manifest> Manifest;
	static std::unique_ptr<Model> Model;
	static std::unique_ptr<EditorApp> EditorApp;


};

