#pragma once

#include "LuaScript.h"

struct ScriptComponent {
	LuaScript Script;

	ScriptComponent(const LuaScriptFile & scriptFile, void * ownerPtr) : Script{ scriptFile, ownerPtr } {

	}
};

