#pragma once

#include "Model.h"
#include "Field.hpp"
#include "NullableField.hpp"

class User : public Model {
public:
	Field<int> Id;
	Field<std::string> Name;
	Field<std::string> Password;
	Field<std::string> Salt;
	NullableField<std::string> Session;
	Field<int> ExpiresAt;
	Field<bool> IsBanned;

	AggregatedField<int> Count;

	void Copy(const User & u) {
		Id = u.Id;
		Name = u.Name;
		Password = u.Password;
		Salt = u.Salt;
		Session = u.Session;
		ExpiresAt = u.ExpiresAt;
		IsBanned = u.IsBanned;
	}

	User(const User & s) : User() {
		Copy(s);
	}

	User & operator=(const User & u) {
		Copy(u);
		return *this;
	}

	User();

	virtual void Save() override;

	static User Find(int id);

	static User FindByHash(const std::string & hash);

	static void CreateUser(const std::string & name, const std::string & password, const std::string & salt = "", int expires_at = 0, bool is_banned = 0);

	static std::vector<User> FetchAll();

	static bool Exists(int id);
};
