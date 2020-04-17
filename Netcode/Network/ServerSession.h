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
		std::shared_ptr<UdpStream> controlStream;
		std::shared_ptr<UdpStream> gameStream;
		PacketStorage storage;
		ControlPacketStorage controlStorage;
		MysqlSession db;
		ErrorCode lastError;
		NetworkContext dbContext;

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
		std::shared_ptr<ServerSession> GetStrongRef();

	public:
		virtual ~ServerSession() override;

		ServerSession(boost::asio::io_context & ioc, Network::Config config);

		virtual void Start() override;

		virtual void Stop() override;

		/*
		messages that are validated protocol wise
		*/
		virtual void Receive(std::vector<Protocol::Message> & control, std::vector<Protocol::Message> & game) override;

		virtual bool IsRunning() const override;

		virtual std::string GetLastError() const override;

		virtual void SendAll() override;

		virtual void SendControlMessage(Protocol::Message message, std::function<void(ErrorCode)> completionHandler) override;

		virtual void SendUpdate(Protocol::Message message) override;

		virtual void SendUpdate(const udp_endpoint_t & endpoint, Protocol::Message message) override;

		template<typename CompletionHandler = DefaultMysqlCompletionHandler>
		void CreateGameSession(int32_t userId, CompletionHandler completionHandler = DefaultMysqlCompletionHandler{}) {
			boost::asio::post(dbContext.GetImpl(), [pThis = GetStrongRef(), userId, completionHandler]() -> void {
				ErrorCode ec = pThis->db.CreateGameSession(userId);
				completionHandler(std::move(ec));
			});
		}

		template<typename CompletionHandler>
		void Authenticate(std::string hash, CompletionHandler completionHandler) {
			boost::asio::post(dbContext.GetImpl(), [pThis = GetStrongRef(), hash, completionHandler]() -> void {
				UserRow ur;
				ErrorCode ec = pThis->db.QueryUserByHash(hash, ur);
				completionHandler(std::move(ec), std::move(ur));
			});
		}

		template<typename CompletionHandler = DefaultMysqlCompletionHandler>
		void TerminateGameSession(int32_t userId, CompletionHandler completionHandler = DefaultMysqlCompletionHandler{}) {
			boost::asio::post(dbContext.GetImpl(), [pThis = GetStrongRef(), userId, completionHandler]() -> void {
				ErrorCode ec = pThis->db.CloseGameSession(userId);
				completionHandler(std::move(ec));
			});
		}
	};

}
