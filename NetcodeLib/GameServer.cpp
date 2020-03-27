#include "GameServer.h"
#include "Mysql.h"

GameServer GameServer::Find(int id) {
	GameServer g;
	Database
		.Select()
		.From<GameServer>()
		.Join(&GameServer::OwnerId, &User::Id)
		.Where("game_servers.id = ?").Bind(id)
		.FetchInto(g);
	return g;
}


GameServer GameServer::Create(int ownerId, int maxPlayers, int interval, int status, const std::string & ip, int createdAt) {
    GameServer g;
    g.OwnerId = ownerId;
    g.MaxPlayers = maxPlayers;
	g.Interval = interval;
    g.Status = status;
    g.ServerIp = ip;
    g.CreatedAt = createdAt;
	g.Save();
	return g;
}

void GameServer::Emplace(int ownerId, int maxPlayers, int interval, int status, const std::string & ip, int createdAt) {
	OwnerId = ownerId;
	MaxPlayers = maxPlayers;
	Interval = interval;
	Status = status;
	ServerIp = ip;
	CreatedAt = createdAt;
	Save();
}

void GameServer::Save() {
	try {
		if(Id == 0) {
			Id = Database.Insert(*this).Execute();
			SetNull(Id == 0);
		} else {
			Database.Update(Id, *this).Execute();
			SetNull(false);
		}
	} catch(mysqlx::Error &err) {

	}
}
