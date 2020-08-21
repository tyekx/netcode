#include "pch.h"
#include "XamlGlobal.h"

std::unique_ptr<Netcode::Asset::Manifest> Global::Manifest{ nullptr };
std::unique_ptr<Netcode::Intermediate::Model> Global::Model{ nullptr };
std::unique_ptr<EditorApp> Global::EditorApp{ nullptr };

