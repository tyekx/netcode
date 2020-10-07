#pragma once

#include <utility>

namespace Netcode::Network {

	template<typename SockType, typename SockReaderWriter>
	class BasicSocket : private SockReaderWriter {
	protected:
		SockType socket;
	public:
		using SocketType = SockType;

		BasicSocket(SocketType s) : SockReaderWriter{ s }, socket{ std::move(s) } {}

		const SockType & GetSocket() const {
			return socket;
		}

		SockType & GetSocket() {
			return socket;
		}

		template<typename ... ARGS>
		auto Send(ARGS && ... args) {
			return SockReaderWriter::Write(socket, std::forward<ARGS>(args)...);
		}

		template<typename ... ARGS>
		auto Receive(ARGS && ... args) {
			return SockReaderWriter::Read(socket, std::forward<ARGS>(args)...);
		}
	};

	template<typename SockType>
	class AsioSocketReadWriter {
	public:
		AsioSocketReadWriter(const SockType &) { }

		template<typename MutableBufferSequence, typename Endpoint, typename Handler>
		static void Read(SockType & socket, const MutableBufferSequence & buffers, Endpoint & remoteEndpoint, Handler && handler) {
			socket.async_receive_from(buffers, remoteEndpoint, std::forward<Handler>(handler));
		}

		template<typename MutableBufferSequence, typename Handler>
		static void Read(SockType & socket, const MutableBufferSequence & buffers, Handler && handler) {
			socket.async_receive(buffers, std::forward<Handler>(handler));
		}

		template<typename ConstBufferSequence, typename Handler>
		static void Write(SockType & socket, const ConstBufferSequence & buffers, Handler && handler) {
			socket.async_send(buffers, handler);
		}

		template<typename ConstBufferSequence, typename Endpoint, typename Handler>
		static void Write(SockType & socket, const ConstBufferSequence & buffers, const Endpoint & remoteEndpoint, Handler && handler) {
			socket.async_send_to(buffers, remoteEndpoint, handler);
		}
	};

	template<typename SockType>
	using BasicAsioSocket = BasicSocket<SockType, AsioSocketReadWriter<SockType>>;

}
