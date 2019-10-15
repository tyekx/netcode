#include "LuaApi.h"
#include "GameObject.h"
#include "Utility.h"
/*
int LuaApi::LUA_API_GetRotation(lua_State * state) {
	Egg::GameObject * pThis = reinterpret_cast<Egg::GameObject *>(Internal::GetInstance(state));


	Egg::Math::Float4 f = Egg::Math::Float4::Identity;
	if(pThis->HasComponent<TransformComponent>()) {
		TransformComponent * tc = pThis->GetComponent<TransformComponent>();
		f = tc->Rotation;
	}

	Internal::PushFloat4(state, f);

	return 1;
}

int LuaApi::LUA_API_SetRotation(lua_State * state) {
	Egg::GameObject * pThis = reinterpret_cast<Egg::GameObject *>(Internal::GetInstance(state));

	Egg::Math::Float4 q = Internal::GetFloat4Arg(state, 1);
	if(pThis->HasComponent<TransformComponent>()) {
		TransformComponent * tc = pThis->GetComponent<TransformComponent>();
		tc->Rotation = q;
	}

	return 0;
}

int LuaApi::LUA_API_Printf(lua_State * state) {
	const char * str = luaL_checkstring(state, 1);

	OutputDebugString(str);
	OutputDebugString("\r\n");

	return 0;
}

void LuaApi::InitAPI(lua_State * state) {
	struct LibData {
		const char * luaName;
		lua_CFunction function;
	};

	static LibData libfunctions[] = {
		{ "Printf", LUA_API_Printf },
		{ "GetRotation", LUA_API_GetRotation },
		{ "SetRotation", LUA_API_SetRotation }
	};

	for(int i = 0; i < _countof(libfunctions); ++i) {
		lua_pushcfunction(state, libfunctions[i].function);
		lua_setglobal(state, libfunctions[i].luaName);
	}
}

std::string LuaApi::Internal::GetStringArg(lua_State * state, int arg) {
	return std::string{ luaL_checkstring(state, arg) };
}

int LuaApi::Internal::GetIntArg(lua_State * state, int arg) {
	return (int)luaL_checkinteger(state, arg);
}

float LuaApi::Internal::GetFloatArg(lua_State * state, int arg) {
	return (float)luaL_checknumber(state, arg);
}

Egg::Math::Float2 LuaApi::Internal::GetFloat2Arg(lua_State * state, int arg) {
	luaL_checktype(state, arg, LUA_TTABLE);

	lua_getfield(state, arg, "x");
	lua_getfield(state, arg, "y");

	float x = GetFloatArg(state, -2);
	float y = GetFloatArg(state, -1);

	lua_pop(state, 2);

	return Egg::Math::Float2{ x, y };
}

Egg::Math::Float3 LuaApi::Internal::GetFloat3Arg(lua_State * state, int arg) {
	luaL_checktype(state, arg, LUA_TTABLE);

	lua_getfield(state, arg, "x");
	lua_getfield(state, arg, "y");
	lua_getfield(state, arg, "z");

	float x = GetFloatArg(state, -3);
	float y = GetFloatArg(state, -2);
	float z = GetFloatArg(state, -1);

	lua_pop(state, 3);

	return Egg::Math::Float3{ x, y, z };
}

Egg::Math::Float4 LuaApi::Internal::GetFloat4Arg(lua_State * state, int arg) {
	luaL_checktype(state, arg, LUA_TTABLE);

	lua_getfield(state, arg, "x");
	lua_getfield(state, arg, "y");
	lua_getfield(state, arg, "z");
	lua_getfield(state, arg, "w");

	float x = GetFloatArg(state, -4);
	float y = GetFloatArg(state, -3);
	float z = GetFloatArg(state, -2);
	float w = GetFloatArg(state, -1);

	lua_pop(state, 4);

	return Egg::Math::Float4{ x, y, z, w };
}

void LuaApi::Internal::PushFloat4(lua_State * state, const Egg::Math::Float4 & f) {
	lua_createtable(state, 0, 4);
	lua_pushstring(state, "x");
	lua_pushnumber(state, f.x);
	lua_settable(state, -3);
	lua_pushstring(state, "y");
	lua_pushnumber(state, f.y);
	lua_settable(state, -3);
	lua_pushstring(state, "z");
	lua_pushnumber(state, f.z);
	lua_settable(state, -3);
	lua_pushstring(state, "w");
	lua_pushnumber(state, f.w);
	lua_settable(state, -3);
}

void * LuaApi::Internal::GetInstance(lua_State * state) {
	lua_getglobal(state, "instancePtr");
	void * p = lua_touserdata(state, -1);
	lua_pop(state, 1);
	return p;
}
*/