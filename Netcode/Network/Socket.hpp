#pragma once

#include <utility>
#include "NetworkDecl.h"
#include <Netcode/Config.h>

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
	class AsioSocketReaderWriter {
	public:
		AsioSocketReaderWriter(const SockType &) { }

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
	class DebugAsioSocketReaderWriter {
		using ExecutorType = typename SockType::executor_type;
		ExecutorType executor;
		Duration fakeLagDuration;
	public:
		DebugAsioSocketReaderWriter(SockType& sock) : executor{ sock.get_executor() } {
			const uint32_t fakeLagMs = Config::GetOptional<uint32_t>(L"network.debugFakeLagMs:u32", 0);
			fakeLagDuration = std::chrono::milliseconds(fakeLagMs);
		}

		template<typename MutableBufferSequence, typename Endpoint, typename Handler>
		static void Read(SockType & socket, const MutableBufferSequence & buffers, Endpoint & remoteEndpoint, Handler && handler) {
			socket.async_receive_from(buffers, remoteEndpoint, std::forward<Handler>(handler));
		}

		template<typename MutableBufferSequence, typename Handler>
		static void Read(SockType & socket, const MutableBufferSequence & buffers, Handler && handler) {
			socket.async_receive(buffers, std::forward<Handler>(handler));
		}

		template<typename ConstBufferSequence, typename Handler>
		void Write(SockType & socket, const ConstBufferSequence & buffers, Handler && handler) {
			if(fakeLagDuration > Duration{}) {
				WaitableTimer * timer = new WaitableTimer{ executor };

				timer->expires_after(fakeLagDuration);
				timer->async_wait([&socket, handler, buffers, timer](ErrorCode ec) mutable {
					if(!ec) {
						socket.async_send(buffers, handler);
					}
					delete timer;
				});
			} else {
				socket.async_send(buffers, handler);
			}
		}

		template<typename ConstBufferSequence, typename Endpoint, typename Handler>
		void Write(SockType & socket, const ConstBufferSequence & buffers, const Endpoint & remoteEndpoint, Handler && handler) {
			if(fakeLagDuration > Duration{}) {
				WaitableTimer * timer = new WaitableTimer{ executor };

				timer->expires_after(fakeLagDuration);
				timer->async_wait([s = &socket, h = std::move(handler), b = buffers, ep = remoteEndpoint, timer](ErrorCode ec) mutable {
					if(!ec) {
						s->async_send_to(b, ep, std::move(h));
					}
					delete timer;
				});
			} else {
				socket.async_send_to(buffers, remoteEndpoint, handler);
			}
		}
	};

	template<typename SockType>
	using BasicAsioSocket = BasicSocket<SockType, AsioSocketReaderWriter<SockType>>;

}
