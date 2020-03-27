	#pragma once

#include "Model.h"
#include "Field.hpp"

#include "GameServer.h"
#include "User.h"

struct Session : public Model {
	Field<int> Id;
	Field<int> GameServerId;
	Field<int> UserId;
	Field<int> JoinedAt;
	NullableField<int> LeftAt;

	GameServer Server;
	User Player;

	AggregatedField<int> CountId;

	Session() : Model{ "sessions" },
		Id{ "id", this, 0, E_PRIMARY_KEY | E_AUTO_INCREMENT },
		GameServerId{ "game_server_id", this, 0, E_FOREIGN_KEY },
		UserId{ "user_id", this, 0, E_FOREIGN_KEY },
		LeftAt{ "left_at", this },
		JoinedAt{ "joined_at", this, (int)time(NULL) },
		CountId{ Id, this } {
		Merge(Player);
		Merge(Server);
	}

	void Copy(const Session &s) {
		Id = s.Id;
		GameServerId = s.GameServerId;
		UserId = s.UserId;
		JoinedAt = s.JoinedAt;
		LeftAt = s.LeftAt;
		Server = s.Server;
		Player = s.Player;
	}

	Session(const Session & s) : Session() {
		Copy(s);
	}

	Session & operator=(const Session &s) {
		Copy(s);
		return *this;
	}

	static bool Has(int userId);

	virtual void Save() override;

	static Session Create(int gameServerId, int userId, int joinedAt) {
		Session s;
		s.GameServerId = gameServerId;
		s.UserId = userId;
		s.JoinedAt = joinedAt;
		s.Save();
		return s;
	}

	static Session Find(int id);


	static void UserLeft(int gameServerId, int userId);

	static bool HasActiveSession(int playerId);

	static int ServerClosed(int gameServerId);
};
