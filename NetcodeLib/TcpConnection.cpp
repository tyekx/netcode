#include "TcpConnection.h"

TcpConnection::TcpConnection() : Socket{ IOEngine::GetIOContext() } {}

boost::asio::ip::tcp::socket & TcpConnection::GetSocket() {
	return Socket;
}

void TcpConnection::Connect(const Endpoint & ep, ConnectHandler & handler) {
	ConnectedEndpoint = boost::asio::ip::tcp::endpoint{ boost::asio::ip::address::from_string(ep.Address), ep.Port };
	Socket.async_connect(ConnectedEndpoint, [this, &handler](const boost::system::error_code & ec) -> void {
		if (ec != boost::system::errc::success) {
			ConnectedEndpoint = boost::asio::ip::tcp::endpoint{};
			handler.ConnectFail(ec.message());
		} else {
			handler.ConnectSuccess();
		}
	});
}

void TcpConnection::Close() {
	Socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
	Socket.close();
}
