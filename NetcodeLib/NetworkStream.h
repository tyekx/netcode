#pragma once

#include "StreamInterfaces.h"
#include "IOEngine.h"

class NetworkStream : public Stream {
protected:
	boost::asio::streambuf ReadBuffer;
	boost::asio::streambuf::mutable_buffers_type MutableBuffer;

	void HandleBuffer(std::string & target, std::size_t size, boost::asio::streambuf & buffer);
	void ResetBuffer();

public:
	NetworkStream();
};

using SharedNetworkStream = std::shared_ptr<NetworkStream>;


