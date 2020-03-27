#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>

#include "Endpoint.h"
#include "StreamMessage.h"

struct ConnectHandler {
	virtual void ConnectSuccess() = 0;
	virtual void ConnectFail(const std::string & error) = 0;
};

struct StreamHandler {
	virtual void ReadSuccess(SharedStreamMessage msg) = 0;
	virtual void ReadFail(const std::string & error) = 0;

	virtual void WriteSuccess(std::size_t sentBytes) = 0;
	virtual void WriteFail(const std::string & error) = 0;

	virtual ~StreamHandler() {}
};

struct Stream {
	virtual void Read(StreamHandler & handler) = 0;
	virtual void Write(SharedStreamMessage data, StreamHandler & handler) = 0;
	virtual void Close() {}

	virtual ~Stream() {}
};
