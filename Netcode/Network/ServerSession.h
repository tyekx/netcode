#pragma once 

#include "NetworkDecl.h"
#include "NetworkCommon.h"
#include "GameSession.h"
#include "MysqlSession.h"
#include "../DestructiveCopyConstructible.hpp"
#include <boost/asio/deadline_timer.hpp>

namespace Netcode::Network {

	class ServerSession : public ServerSessionBase {
		boost::asio::io_context & ioContext;
		boost::asio::deadline_timer timer;
		MessageQueue<UdpPacket> gameQueue;
		Ref<UdpStream> gameStream;
		Ref<PacketStorage> storage;
		MysqlSession db;
		NetworkContext dbContext;

		ErrorCode ConnectToMysql();
		void GameSocketReadInit();
		void OnGameRead(UdpPacket packet);
		void OnMessageSent(const ErrorCode & ec, std::size_t size);
		void ParseGameMessages(std::vector<Protocol::ClientUpdate> & outVec, std::vector<UdpPacket> packets);

	public:
		virtual ~ServerSession() override;

		ServerSession(boost::asio::io_context & ioc);

		virtual void Start() override;

		virtual void Stop() override;

		virtual void SwapBuffers(std::vector<Protocol::ClientUpdate> & game) override {
			std::vector<UdpPacket> packets;
			gameQueue.GetIncomingPackets(packets);

			ParseGameMessages(game, std::move(packets));
		}
		
		virtual void Update(int32_t subjectId, Protocol::ServerUpdate serverUpdate) override;

		template<typename CompletionHandler = DefaultMysqlCompletionHandler>
		void CreateGameSession(int32_t userId, CompletionHandler completionHandler = DefaultMysqlCompletionHandler{}) {
			boost::asio::post(dbContext.GetImpl(), [this, userId, completionHandler, lifetime = shared_from_this()]() -> void {
				ErrorCode ec = db.CreateGameSession(userId);
				completionHandler(std::move(ec));
			});
		}

		template<typename CompletionHandler>
		void Authenticate(std::string hash, CompletionHandler completionHandler) {
			boost::asio::post(dbContext.GetImpl(), [this, hash, completionHandler, lifetime = shared_from_this()]() -> void {
				PlayerDbDataRow ur;
				ErrorCode ec = db.QueryUserByHash(hash, ur);
				completionHandler(ec, ur);
			});
		}

		template<typename CompletionHandler = DefaultMysqlCompletionHandler>
		void TerminateGameSession(int32_t userId, CompletionHandler completionHandler = DefaultMysqlCompletionHandler{}) {
			boost::asio::post(dbContext.GetImpl(), [this, userId, completionHandler, lifetime = shared_from_this()]() -> void {
				ErrorCode ec =  db.CloseGameSession(userId);
				completionHandler(std::move(ec));
			});
		}
	};

}
