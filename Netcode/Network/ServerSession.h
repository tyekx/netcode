#pragma once 

#include "NetworkDecl.h"
#include "NetworkCommon.h"
#include "GameSession.h"
#include "MysqlSession.h"
#include "../DestructiveCopyConstructible.hpp"
#include <boost/asio/deadline_timer.hpp>
#include <Netcode/Logger.h>

namespace Netcode::Network {

	class ConnectionStorage {
		mutable SlimReadWriteLock srwLock;
		std::vector<Ref<Connection>> connections;
	public:
		NETCODE_CONSTRUCTORS_DELETE_MOVE(ConnectionStorage);
		NETCODE_CONSTRUCTORS_DELETE_COPY(ConnectionStorage);

		ConnectionStorage() : srwLock{}, connections{} {
			connections.reserve(32);
		}

		void AddConnection(Ref<Connection> connection) {
			if(connection == nullptr) {
				return;
			}
			
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			connections.emplace_back(std::move(connection));
		}

		[[nodiscard]]
		Ref<Connection> GetConnectionByEndpoint(const UdpEndpoint & ep) {
			ScopedSharedLock<SlimReadWriteLock> sharedScopedLick{ srwLock };

			const auto it = std::find_if(std::cbegin(connections), std::cend(connections), [&ep](const Ref<Connection> & conn) -> bool {
				return conn->liveEndpoint == ep;
			});

			if(it != std::cend(connections)) {
				return *it;
			}

			return nullptr;
		}

		void RemoveConnection(Ref<Connection> connection) {
			if(connection == nullptr) {
				return;
			}

			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			const auto it = std::find(std::cbegin(connections), std::cend(connections), connection);

			if(it != std::cend(connections)) {
				connections.erase(it);
			}
		}

		[[nodiscard]]
		Ref<Connection> GetConnectionByNonce(const std::string & nonce) const {
			ScopedSharedLock<SlimReadWriteLock> sharedScopedLick{ srwLock };

			const auto it = std::find_if(std::cbegin(connections), std::cend(connections), [&nonce](const Ref<Connection> & conn) -> bool {
				return conn->nonce == nonce;
			});

			if(it != std::cend(connections)) {
				return *it;
			}
			
			return nullptr;
		}

		[[nodiscard]]
		Ref<Connection> GetConnectionById(int32_t id) const {
			ScopedSharedLock<SlimReadWriteLock> sharedScopedLick{ srwLock };

			const auto it = std::find_if(std::cbegin(connections), std::cend(connections), [id](const Ref<Connection> & conn) -> bool {
				return std::get<0>(conn->playerData) == id;
			});

			if(it != std::cend(connections)) {
				return *it;
			}

			return nullptr;
		}
		
	};

	class ServerSession : public ServerSessionBase {
		boost::asio::io_context & ioContext;
		boost::asio::deadline_timer timer;
		MessageQueue<Protocol::ClientUpdate> gameQueue;
		Ref<PacketStorage<UdpPacket>> storage;
		Ref<Connection> connection;
		MysqlSession db;
		NetworkContext dbContext;
		ConnectionStorage connectionStorage;

		ErrorCode ConnectToMysql();
		void GameSocketReadInit();
		void OnGameRead(Ref<UdpPacket> packet);
		void OnMessageSent(const ErrorCode & ec, std::size_t size);

	public:
		virtual ~ServerSession() override;

		ServerSession(boost::asio::io_context & ioc);

		virtual void Start() override;

		virtual void Stop() override;

		virtual uint16_t GetPort() const override {
			return connection->liveEndpoint.port();
		}

		virtual Ref<Connection> GetConnectionById(int32_t userId) override {
			return connectionStorage.GetConnectionById(userId);
		}
		
		virtual Ref<Connection> GetConnectionByNonce(const std::string & nonce) override {
			return connectionStorage.GetConnectionByNonce(nonce);
		}
		
		virtual Ref<Connection> MakeEmptyConnection() override;
		
		virtual void ActivateConnection(Ref<Connection> activeConnection) override {
			activeConnection->state = ConnectionState::CONNECTING;
			connectionStorage.AddConnection(std::move(activeConnection));
		}

		virtual void RemoveConnection(Ref<Connection> connection) override {
			connectionStorage.RemoveConnection(std::move(connection));
		}

		virtual void SwapBuffers(std::vector<Protocol::ClientUpdate> & game) override {
			gameQueue.GetIncomingPackets(game);
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
