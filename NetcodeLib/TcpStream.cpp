#include "TcpStream.h"


TcpStream::TcpStream(SharedTcpConnection conn) : NetworkStream{}, TcpConnection{ conn } { }

void TcpStream::Read(StreamHandler & handler) {
	boost::asio::async_read(TcpConnection->GetSocket(), MutableBuffer, boost::asio::transfer_at_least(1), [this, &handler](const boost::system::error_code & ec, size_t readData) -> void {
		if (ec != boost::system::errc::success) {
			handler.ReadFail(ec.message());
		} else {
			SharedStreamMessage msg = std::make_shared<StreamMessage>();
			msg->Subject.reset(new Endpoint{});
			msg->Subject->Address = TcpConnection->GetSocket().remote_endpoint().address().to_string();
			msg->Subject->Port = TcpConnection->GetSocket().remote_endpoint().port();
			msg->Bytes = readData;
			HandleBuffer(msg->Data, msg->Bytes, ReadBuffer);
			handler.ReadSuccess(msg);
		}
		ResetBuffer();
	});
}

void TcpStream::Write(SharedStreamMessage data, StreamHandler & handler) {
	boost::asio::async_write(TcpConnection->GetSocket(), boost::asio::buffer(data->Data), [this, data, &handler](const boost::system::error_code & ec, size_t sentData) -> void {
		if (ec != boost::system::errc::success) {
			handler.WriteFail(ec.message());
		} else {
			handler.WriteSuccess(sentData);
		}
	});
}

void TcpStream::Close()  {
	TcpConnection->Close();
}
