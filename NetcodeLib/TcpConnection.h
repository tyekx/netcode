#pragma once

#include "StreamInterfaces.h"
#include "IOEngine.h"

class TcpConnection {
protected:
	boost::asio::ip::tcp::socket Socket;
	boost::asio::ip::tcp::endpoint ConnectedEndpoint;

public:
	TcpConnection();

	boost::asio::ip::tcp::socket & GetSocket();
	void Connect(const Endpoint & ep, ConnectHandler & handler);
	void Close();
};

using SharedTcpConnection = std::shared_ptr<TcpConnection>;
