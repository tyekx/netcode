#include "UdpStream.h"
#include "Config.h"

UdpStream::UdpStream(unsigned short port) :
		NetworkStream{},
		LocalEndpoint{ boost::asio::ip::address::from_string(Config::Get<std::string>("LocalUdpBindAddress")), port },
		Socket{ IOEngine::GetIOContext(), LocalEndpoint },
		RemoteReceiveFromEndpoint{},
		RemoteSendToEndpoint{} {

}

#ifdef _MSC_VER
//windows
#define ERR_VALUE 995
#else
//linux
#define ERR_VALUE 995
#endif

void UdpStream::Read(StreamHandler & handler) {
	Socket.async_receive_from(MutableBuffer, RemoteReceiveFromEndpoint, [this, &handler](const boost::system::error_code & ec, std::size_t readBytes) -> void {
		if (ec != boost::system::errc::success) {
			std::cout << ec.category().name() << std::endl;
			std::cout << ec.value() << std::endl;
			if(ec.value() != ERR_VALUE) {
				handler.ReadFail(ec.message());
			} else {
				return;
			}
		} else {
			SharedStreamMessage msg = std::make_shared<StreamMessage>();
			msg->Subject.reset(new Endpoint{});
			msg->Subject->Address = RemoteReceiveFromEndpoint.address().to_string();
			msg->Subject->Port = RemoteReceiveFromEndpoint.port();
			msg->Bytes = readBytes;
			HandleBuffer(msg->Data, msg->Bytes, ReadBuffer);
			MutableBuffer = ReadBuffer.prepare(8196);
			handler.ReadSuccess(msg);
		}
		ResetBuffer();
	});
}

void UdpStream::Write(SharedStreamMessage data, StreamHandler & handler) {
	if (data->Subject == nullptr) {
		return;
	}
	RemoteSendToEndpoint = boost::asio::ip::udp::endpoint{ boost::asio::ip::address::from_string(data->Subject->Address), data->Subject->Port };
	Socket.async_send_to(boost::asio::buffer(data->Data), RemoteSendToEndpoint, [this, data, &handler](const boost::system::error_code & ec, std::size_t size) -> void {
		if (ec != boost::system::errc::success) {
			handler.WriteFail(ec.message());
			return;
		} else {
			handler.WriteSuccess(size);
		}
	});
}

