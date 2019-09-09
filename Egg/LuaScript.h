#pragma once

#include <functional>
#include <lua/lua.hpp>
#include <string>
#include "Utility.h"
#include "LuaApi.h"
#include "LuaScriptFile.h"

class LuaScript {
	lua_State * State;

public:
	/*
	void TableForeach(int index, std::function<void(const std::string &)> f) {
		lua_pushnil(State);
		while(lua_next(State, index) != 0) {
			
			if(lua_type(State, -2) != LUA_TSTRING && lua_type(State, -2) != LUA_TNUMBER) {
				OutputDebugString("LUA script error, key is not string nor number\r\n");
			}

			std::string key;

			int t = lua_type(State, -2);

			if(t == LUA_TNUMBER) {
				lua_Number ln = lua_tonumber(State, -2);
				int i = (int)ln;
				key = std::to_string(i);
			} else if(t == LUA_TSTRING) {
				key = lua_tostring(State, -2);
			}

			f(key);

			lua_pop(State, 1);
		}
	}

	void PrintValue() {
		switch(lua_type(State, -1)) {
		case LUA_TTABLE:
			TableForeach(lua_gettop(State), [this](const std::string & s) -> void {
				OutputDebugString(s.c_str());
				OutputDebugString(": ");
				PrintValue();
			});
			break;
		case LUA_TNUMBER:
			OutputDebugString(std::to_string(lua_tonumber(State, -1)).c_str());
			OutputDebugString("\r\n");
			break;
		case LUA_TSTRING:
			OutputDebugString(lua_tostring(State, -1));
			OutputDebugString("\r\n");
			break;
		default:
			OutputDebugString("Unknown LUA type\r\n");
			break;
		}
	}*/

	LuaScript(const LuaScriptFile & scriptContent,  void * objPtr) {
		State = luaL_newstate();

		if(luaL_loadstring(State, scriptContent.Content.c_str()) != LUA_OK) {
			Egg::Utility::Debugf("Failed to load LUA script: '%s'\r\nError:\r\n%s", scriptContent.File.c_str(), lua_tostring(State, -1));
			return;
		}
		
		LuaApi::InitAPI(State);

		if(lua_pcall(State, 0, 0, 0) != LUA_OK) {
			Egg::Utility::Debugf("Failed to run LUA script: '%s'\r\nError:\r\n%s", scriptContent.File.c_str(), lua_tostring(State, -1));
			return;
		}

		static const char * luName = "instancePtr";

		lua_pushlightuserdata(State, objPtr);
		lua_setglobal(State, luName);

		lua_pop(State, 1);
	}

	void InvokeInit() {
		lua_getglobal(State, "init");
		lua_pushnil(State);

		if(lua_pcall(State, 1, 0, 0) != LUA_OK) {
			Egg::Utility::Debugf("Failed to run LUA script, Error:\r\n%s", lua_tostring(State, -1));
		}
	}

	void InvokeUpdate(float dt) {
		lua_getglobal(State, "update");
		lua_pushnumber(State, dt);

		if(lua_pcall(State, 1, 0, 0) != LUA_OK) {
			Egg::Utility::Debugf("Failed to run LUA script, Error: %s\r\n", lua_tostring(State, -1));
		}
	}

	~LuaScript() {
		if(State) {
			lua_close(State);
		}
	}

};
