#pragma once

#include <iostream>
#include <string>
#include <memory>

struct Endpoint {
	std::string Address;
	unsigned short Port;

	Endpoint();

	bool Equals(const Endpoint & ep) {
		return Address == ep.Address && Port == ep.Port;
	}
};

using SharedEndpoint = std::shared_ptr<Endpoint>;
