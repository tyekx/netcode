#pragma once

#include <Netcode/Modules.h>
#include <map>
#include <string>
#include <Netcode/Vertex.h>
#include <Netcode/Utility.h>
#include "GameObject.h"
#include "Mesh.h"

using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceState;

struct GameServerData {
	int rtt;
	int activePlayers;
	int availableSlots;
	int port;
	std::string hostname;
	std::string address;
	std::string host;
};
