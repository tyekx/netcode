#include "User.h"
#include "Mysql.h"
#include <exception>

User::User() : Model("users"),
	Id{ "id", this, 0, E_AUTO_INCREMENT | E_PRIMARY_KEY },
	Name{ "name", this },
	Password{ "password", this },
	Salt{ "salt", this },
	Session{ "session", this },
	ExpiresAt{ "expires_at", this },
	IsBanned{ "is_banned", this },
	Count{ Id, this } { }

User User::Find(int id) {
	User u;
	Database
		.Select()
		.From<User>()
		.Where("id = ?")
		.Bind(id)
		.Debug()
		.FetchInto(u);
	return u;
}

void User::Save() {
	if(Id == 0) {
		Id = Database.Insert(*this).Execute();
	} else {
		Database.Update(Id, *this).Execute();
	}
}

bool User::Exists(int id) {
	User u;
	Database
		.Select()
		.Count(&User::Count)
		.From<User>()
		.Where("id = ?")
		.Bind(id)
		.FetchInto(u);
	if(u) {
		return u.Count == 1;
	} else return false;
}

std::vector<User> User::FetchAll() {
	std::vector<User> users;
	Database.Select(&User::Name, &User::Password).From<User>().FetchInto(users);
	return users;
}

User User::FindByHash(const std::string & hash) {
	User u;
	Database.Select().From<User>().Where("session = ?").Bind(hash).FetchInto(u);
	return u;
}

void User::CreateUser(const std::string & name,
					  const std::string & password, 
					  const std::string & salt, int expires_at, bool is_banned) {
	User user;
	user.Name = name;
	user.Password = password;
	user.Salt = salt;
	user.ExpiresAt = expires_at;
	user.IsBanned = is_banned;
	user.Save();
}
