#pragma once

#include <lua/lua.hpp>
#include <vector>
#include <string>
#include "Common.h"
#include "Utility.h"
#include "Path.h"
#include "LuaScriptFile.h"

class LuaManager {
	std::vector<LuaScriptFile> Files;
public:
	const LuaScriptFile & GetScript(const std::wstring & key) {
		for(const LuaScriptFile & i : Files) {
			if(i.Name == key || i.File == key) {
				return i;
			}
		}
		throw 1;
	}

	LuaManager() {
		WIN32_FIND_DATAW findData;

		std::wstring folder = Egg::Path::GetMediaRoot() + L"*.lua";

		HANDLE hFind = FindFirstFileW(folder.c_str(), &findData);

		ASSERT(hFind != INVALID_HANDLE_VALUE, "LuaManager: failed to list files from directory '%S'", folder.c_str());

		do {
			std::wstring fileName{ findData.cFileName };

			if(fileName == L"." || fileName == L"..") {
				continue;
			}

			LuaScriptFile lsf;

			size_t indexOf = fileName.find_last_of('.');

			lsf.File = fileName;
			lsf.Name = fileName.substr(0, indexOf);

			Egg::Utility::SlurpFile(lsf.Content, Egg::Path::GetMediaRoot() + fileName);

			Files.push_back(std::move(lsf));
		} while(FindNextFileW(hFind, &findData) != 0);

		DWORD dwError = GetLastError();
		ASSERT(dwError == ERROR_NO_MORE_FILES, "LuaManager: failed to list files from directory '%S'", folder.c_str());
	}
};
