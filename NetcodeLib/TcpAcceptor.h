#pragma once

#include "TcpConnection.h"

struct AcceptHandler {
	virtual void AcceptSuccess(SharedTcpConnection conn) = 0;
	virtual void AcceptFail(const std::string & error) = 0;
};

class TcpAcceptor {
	boost::asio::ip::tcp::acceptor Acceptor;

public:
	TcpAcceptor(unsigned short port);
	void Accept(AcceptHandler & handler);
	void Close();
};
