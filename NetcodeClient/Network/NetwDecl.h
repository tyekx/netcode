#pragma once

#include <Netcode/HandleDecl.h>
#include <string>

struct PlayerStatEntry {
	uint32_t id;
	int32_t kills;
	int32_t deaths;
	int32_t ping;
	std::string name;
};
