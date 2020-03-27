#include "Session.h"
#include "Mysql.h"

Session Session::Find(int id) {
	Session s;

	Database.Select()
		.From<Session>()
		.Join(&Session::UserId, &User::Id)
		.Join(&Session::GameServerId, &GameServer::Id)
		.Where("sessions.id = ?").Bind(id)
		.FetchInto(s);

	return s;
}


bool Session::Has(int userId) {
	Session s;
	if(Database.IsConnected()) {
		Database
			.Select()
			.Count(&Session::CountId)
			.From<Session>()
			.Join(&Session::GameServerId, &GameServer::Id)
			.Join(&Session::UserId, &User::Id)
			.Where("user_id = ? AND left_at IS NULL AND game_servers.status = 'INGAME'")
			.Bind(userId)
			.FetchInto(s);
		return s.CountId != 0;
	} else return false;
}

void Session::Save() {
	try {
		if(Id == 0) {
			Id = Database.Insert(*this).Execute();
			SetNull(Id == 0);
		} else {
			Database.Update(Id, *this).Execute();
			SetNull(false);
		}
	} catch(mysqlx::Error & err) {
	}
}

bool Session::HasActiveSession(int playerId) {
	Session s;

	Database.Select().Count(&Session::CountId).Where("user_id = ? AND left_at IS NULL").Bind(playerId).FetchInto(s);

	return s.CountId == 1;
}

int Session::ServerClosed(int gameServerId) {
	try {
		int i = Database.Update("sessions", &Session::LeftAt)
			.Set((int)time(NULL))
			.Where("game_server_id = ? AND left_at IS NULL")
			.Bind(gameServerId)
			.Debug()
			.Execute();
		return i;
	} catch(mysqlx::Error & err) {
	}
	return 0;
}


void Session::UserLeft(int gameServerId, int userId) {
	try {
		Database
			.Update("sessions", &Session::LeftAt)
			.Set((int)time(NULL))
			.Where("game_server_id = ? AND user_id = ? AND left_at IS NULL")
			.Bind(gameServerId, userId)
			.Safe()
			.Debug()
			.Execute();
	} catch(mysqlx::Error & error) {
	}
}
