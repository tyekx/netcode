#include "NetworkStream.h"

NetworkStream::NetworkStream() : ReadBuffer{}, MutableBuffer{ ReadBuffer.prepare(8192) }{

}

void NetworkStream::ResetBuffer() {
	MutableBuffer = ReadBuffer.prepare(8196);
}

void NetworkStream::HandleBuffer(std::string & target, std::size_t size, boost::asio::streambuf & buffer) {
	target.clear();
	target.reserve(size + 1);
	buffer.commit(size);
	target.assign(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + size);
	buffer.consume(size);
}
