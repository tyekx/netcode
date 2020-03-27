#include "TcpAcceptor.h"

TcpAcceptor::TcpAcceptor(unsigned short port) :
	Acceptor{ IOEngine::GetIOContext(), boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4::any(), port} } {

}

void TcpAcceptor::Accept(AcceptHandler & handler) {
	SharedTcpConnection conn = std::make_shared<TcpConnection>();
	Acceptor.async_accept(conn->GetSocket(), [this, conn, &handler](const boost::system::error_code &ec) -> void {
		if (ec != boost::system::errc::success) {
			handler.AcceptFail(ec.message());
		} else {
			handler.AcceptSuccess(conn);
		}
	});
}

void TcpAcceptor::Close() {
	Acceptor.close();
}
