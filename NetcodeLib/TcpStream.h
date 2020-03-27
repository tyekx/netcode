#pragma once

#include "NetworkStream.h"
#include "TcpConnection.h"

class TcpStream : public NetworkStream {
protected:
	SharedTcpConnection TcpConnection;

public:
	TcpStream(SharedTcpConnection conn);

	virtual void Read(StreamHandler & handler) override;
	virtual void Write(SharedStreamMessage data, StreamHandler & handler) override;
	virtual void Close() override;
};
