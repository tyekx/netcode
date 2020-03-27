#pragma once

#include "User.h"
#include <ctime>

struct GameServer : public Model {
	Field<int> Id;
	Field<int> OwnerId;
	Field<int> MaxPlayers;
	Field<int> Interval;
	Field<int> Status;
	Field<std::string> ServerIp;
	Field<int> CreatedAt;
	NullableField<int> ClosedAt;

	User Owner;

	GameServer() : Model{ "game_servers" },
		Id{"id", this, 0, E_AUTO_INCREMENT | E_PRIMARY_KEY },
		OwnerId{ "owner_id", this, 0, E_FOREIGN_KEY },
		MaxPlayers{ "max_players", this },
		Interval{ "interval", this },
		Status{ "status", this },
		ServerIp{ "server_ip", this },
		CreatedAt{ "created_at", this, (int)time(NULL) },
		ClosedAt{ "closed_at", this } {
		Merge(Owner);
	}

	void Copy(const GameServer &g) {
		SetNull(!g);
		Id = g.Id;
		OwnerId = g.OwnerId;
		MaxPlayers = g.MaxPlayers;
		Interval = g.Interval;
		Status = g.Status;
		ServerIp = g.ServerIp;
		CreatedAt = g.CreatedAt;
		ClosedAt = g.ClosedAt;
	}

	GameServer(const GameServer & g) : GameServer() {
		Copy(g);
	}

	GameServer & operator=(const GameServer & g) {
		Copy(g);
		return *this;
	}

	void Emplace(int ownerId, int maxPlayers, int interval, int status, const std::string & ip, int createdAt);

	virtual void Save() override;

	static GameServer Find(int id);

	static GameServer Create(int ownerId, int maxPlayers, int interval, int status, const std::string & ip, int createdAt);
};

