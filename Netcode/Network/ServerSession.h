#pragma once 

#include <boost/asio/deadline_timer.hpp>

#include "NetworkCommon.h"
#include "GameSession.h"
#include "MysqlSession.h"
#include "../DestructiveCopyConstructible.hpp"

namespace Netcode::Network {

	class ServerSession : public GameSession {
		boost::asio::io_context & ioContext;
		boost::asio::deadline_timer timer;
		Network::Config config;
		MessageQueue<UdpPacket> controlQueue;
		MessageQueue<UdpPacket> gameQueue;
		std::unique_ptr<UdpStream> controlStream;
		std::unique_ptr<UdpStream> gameStream;
		PacketStorage storage;
		ControlPacketStorage controlStorage;
		MysqlSession db;
		ErrorCode lastError;

		uint32_t BindToPort(udp_socket_t & socket, uint16_t portHint);

		void ConnectToMysql();

		void ControlSocketReadInit();
		void GameSocketReadInit();

		void OnGameRead(UdpPacket packet);
		void OnControlRead(UdpPacket packet);

		void OnMessageSent(const ErrorCode & ec, std::size_t size, PacketStorage::StorageType buffer);

		void ParseControlMessages(std::vector<Protocol::Message> & outVec, std::vector<UdpPacket> packets);

		void ParseGameMessages(std::vector<Protocol::Message> & outVec, std::vector<UdpPacket> packets);

		void SendAck(udp_endpoint_t endpoint, int32_t ack);
	public:
		virtual ~ServerSession() override;

		ServerSession(boost::asio::io_context & ioc, Network::Config config);

		/*
		messages that are validated protocol wise
		*/
		virtual void Receive(std::vector<Protocol::Message> & control, std::vector<Protocol::Message> & game) override;

		virtual bool IsRunning() const override {
			return !((bool)lastError);
		}

		virtual std::string GetLastError() const override {
			return lastError.message();
		}

		template<typename CompletionHandler>
		void CreateGameSession(int32_t userId, CompletionHandler completionHandler) {
			boost::asio::post(ioContext, [this, userId, completionHandler]() -> void {
				ErrorCode ec = db.CreateGameSession(userId);
				completionHandler(std::move(ec));
			});
		}

		template<typename CompletionHandler>
		void Authenticate(std::string hash, CompletionHandler completionHandler) {
			boost::asio::post(ioContext, [this, hash, completionHandler]() -> void {
				UserRow ur;
				ErrorCode ec = db.QueryUserByHash(hash, ur);
				completionHandler(std::move(ec), std::move(ur));
			});
		}

		virtual void SendAll() override;

		virtual std::future<ErrorCode> SendControlMessage(Protocol::Message message) override {
			std::promise<ErrorCode> errorPromise;
			std::future<ErrorCode> errorFuture = errorPromise.get_future();

			if(!message.has_control()) {
				errorPromise.set_value(boost::asio::error::make_error_code(boost::asio::error::invalid_argument));
				Log::Warn("[Network] [Server] Can only send control messages through the control socket");
				return errorFuture;
			}

			UdpPacket packet = storage.GetBuffer();
			packet.endpoint = udp_endpoint_t{
				boost::asio::ip::address::from_string(message.address()),
				static_cast<uint16_t>(message.port())
			};
			int32_t messageId = message.id();

			if(!message.SerializeToArray(packet.mBuffer.data(), packet.mBuffer.size())) {
				errorPromise.set_value(boost::asio::error::make_error_code(boost::asio::error::message_size));
				Log::Error("[Network] [Server] Failed to serialize control message");
				return errorFuture;
			}

			packet.mBuffer = boost::asio::buffer(packet.data.get(), message.ByteSizeLong());

			return controlStorage.Push(messageId, std::move(packet));
		}


		virtual void SendUpdate(Protocol::Message message) override {
			Log::Warn("[Network] [Server] operation is not supported");
		}

		virtual void SendUpdate(const udp_endpoint_t & endpoint, Protocol::Message message) override {
			if(!message.has_server_update()) {
				Log::Warn("[Network] [Server] Can only send server updates on the game socket");
				return;
			}

			UdpPacket packet = storage.GetBuffer();
			packet.endpoint = endpoint;

			if(!message.SerializeToArray(packet.mBuffer.data(), packet.mBuffer.size())) {
				Log::Error("[Network] [Server] Failed to serialize game message");
				lastError = boost::asio::error::make_error_code(boost::asio::error::message_size);
				return;
			}

			packet.mBuffer = boost::asio::buffer(packet.data.get(), message.ByteSizeLong());

			gameQueue.Send(packet);
		}
	};

}
