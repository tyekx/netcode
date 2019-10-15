#pragma once

#include <lua/lua.hpp>
#include <string>

namespace LuaApi {

	namespace Internal {

		std::string GetStringArg(lua_State * state, int arg);

		int GetIntArg(lua_State * state, int arg);

		float GetFloatArg(lua_State * state, int arg);

		/*
		Egg::Math::Float2 GetFloat2Arg(lua_State * state, int arg);

		Egg::Math::Float3 GetFloat3Arg(lua_State * state, int arg);

		Egg::Math::Float4 GetFloat4Arg(lua_State * state, int arg);

		void PushFloat4(lua_State * state, const Egg::Math::Float4 & f);*/

		void * GetInstance(lua_State * state);
	}

	int LUA_API_GetRotation(lua_State * state);
	
	int LUA_API_SetRotation(lua_State * state);

	int LUA_API_Printf(lua_State * state);

	void InitAPI(lua_State * state);

}
