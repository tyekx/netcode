#pragma once

#include "NetworkStream.h"

class UdpStream : public NetworkStream {
	boost::asio::ip::udp::endpoint LocalEndpoint;
	boost::asio::ip::udp::socket Socket;
	boost::asio::ip::udp::endpoint RemoteReceiveFromEndpoint;
	boost::asio::ip::udp::endpoint RemoteSendToEndpoint;
public:
	UdpStream(unsigned short port);

	virtual void Read(StreamHandler & handler) override;
	virtual void Write(SharedStreamMessage data, StreamHandler & handler) override;
};

using SharedUdpStream = std::shared_ptr<UdpStream>;
