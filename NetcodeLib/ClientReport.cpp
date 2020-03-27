#include "ClientReport.h"

int netcode::ClientReport::GetUserId() const {
	return UserId;
}

void netcode::ClientReport::SetUserId(int userId) {
	UserId = userId;
}

netcode::ClientReport::ClientReport() : BReport(), UserId{ 0 } {}

Json::object netcode::ClientReport::CreateStatistics() const {
	Json::object obj = BReport::CreateStatistics();
	obj["id"] = Json::object{ { "decimals", 0 }, {"dimension","none"}, {"dimension_short",""}, {"value", GetUserId()} };
	return obj;
}

#include <iostream>

bool netcode::ClientReport::Deserialize(const Json & json) {
	std::string shapeCheckErr;
	Json::shape shape = ClientReport::ShapeOf();

	if(!json.has_shape(shape, shapeCheckErr)) {
		return false;
	}


	if(!BReport::Deserialize(json)) {
		return false;
	}

	SetUserId(json["user_id"].int_value());

	return true;
}

Json::object netcode::ClientReport::Serialize() const {
	Json::object obj = BReport::Serialize();

	obj["user_id"] = UserId;

	return obj;
}

const Json::shape & netcode::ClientReport::ShapeOf() {
	const static Json::shape shape = {
		{"user_id", Json::Type::NUMBER }
	};

	return shape;
}

