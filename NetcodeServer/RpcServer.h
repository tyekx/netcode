#pragma once

#include <memory>
#include <NetcodeProtocol/netcode.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <Netcode/Network/ServerSession.h>
#include <Netcode/Logger.h>

using npss = Netcode::Protocol::ServerService;
namespace np = Netcode::Protocol;

class RpcSessionBase {
protected:
	enum class SessionStatus : uint32_t { CREATE, PROCESS, FINISH };

	SessionStatus status;

	void Close() {
		delete this;
	}

public:

	RpcSessionBase() : status{ SessionStatus::CREATE } { } 
	virtual ~RpcSessionBase() = default;
	virtual void Create() = 0;
	virtual void Process() = 0;

	void Run() {
		switch(status) {
			case SessionStatus::CREATE: Create(); break;
			case SessionStatus::PROCESS: Process(); break;
			case SessionStatus::FINISH: Close(); break;
		}
	}
};

template<typename ServiceType, typename RequestType, typename ResponseType>
class NetcodeRpcSessionBase : public RpcSessionBase {
protected:
	
	ServiceType * asyncService;
	grpc::ServerCompletionQueue * completionQueue;
	grpc::ServerContext context;
	grpc::ServerAsyncResponseWriter<np::Response> responder;
	Ref<Netcode::Network::ServerSession> serverSession;
	RequestType request;
	ResponseType response;
	
public:
	NetcodeRpcSessionBase(ServiceType * asyncService, grpc::ServerCompletionQueue * cq, Ref<Netcode::Network::ServerSession> serverSession) : RpcSessionBase{},
		asyncService{ asyncService }, completionQueue{ cq }, context{  }, responder{ &context }, serverSession{ std::move(serverSession) },
		request{}, response{} { }
};

class NetcodeRpcSession : public NetcodeRpcSessionBase<npss::AsyncService, np::Request, np::Response> {
protected:

	void WrapSession(const grpc::Status & statusCode) {
		responder.Finish(response, statusCode, this);
		status = SessionStatus::FINISH;
	}
	
public:
	using NetcodeRpcSessionBase::NetcodeRpcSessionBase;
};

class NetcodeRpcConnection : public NetcodeRpcSession {
	Netcode::Network::PlayerDbDataRow playerData;
private:
	void CreateGameSession(int32_t userId) {
		serverSession->CreateGameSession(userId, [this](const Netcode::Network::ErrorCode & ec) -> void {
			if(ec) {
				WrapSession(grpc::Status{ grpc::StatusCode::ALREADY_EXISTS, "Player already has an active connection" });
			} else {
				// generate nonce
				auto * connData = response.mutable_connect_data();
				std::string nonce = "tmp";
				connData->set_nonce(nonce);
				connData->set_port(static_cast<int32_t>(serverSession->GetPort()));

				Ref<Netcode::Network::Connection> tmpConnection = serverSession->MakeEmptyConnection();
				tmpConnection->authorizedAt = Netcode::SystemClock::LocalNow();
				tmpConnection->lastReceivedAt = tmpConnection->authorizedAt;
				tmpConnection->nonce = std::move(nonce);
				tmpConnection->packetSequenceId = 1;
				tmpConnection->peerPacketSequenceId = 0;
				tmpConnection->playerData = std::move(playerData);
				serverSession->ActivateConnection(std::move(tmpConnection));

				Log::Info("[Net] [RPC] client connection activated");
				
				WrapSession(grpc::Status::OK);
			}
		});
	}

	void AuthenticateUser(std::string hash) {
		serverSession->Authenticate(std::move(hash),
			[this](const Netcode::Network::ErrorCode & ec,
				const Netcode::Network::PlayerDbDataRow & ur) -> void {

			playerData = ur;

			response.set_type(request.type());

			if(ec) {
				WrapSession(grpc::Status{ grpc::StatusCode::NOT_FOUND, "Player session was not found" });
			} else {
				int32_t userId = std::get<0>(playerData);

				if(userId > 0) {
					CreateGameSession(userId);
				} else {
					WrapSession(grpc::Status{ grpc::StatusCode::INTERNAL, "Query succeeded but got an invalid player ID" });
				}
			}

		});
	}
	
public:
	using NetcodeRpcSession::NetcodeRpcSession;
	
	virtual void Create() override {
		asyncService->RequestConnect(&context, &request, &responder, completionQueue, completionQueue, this);
		status = SessionStatus::PROCESS;
	}

	virtual void Process() override {
		(new NetcodeRpcConnection(asyncService, completionQueue, serverSession))->Run();

		auto mData = context.client_metadata();
		const auto it = mData.find("x-netcode-auth");
		
		if(it != mData.end()) {
			grpc::string_ref value = it->second;
			AuthenticateUser(std::string{ value.data(), value.size() });
		} else {
			WrapSession(grpc::Status{ grpc::StatusCode::INVALID_ARGUMENT, "Malformed request" });
		}
	}
};

class NetcodeRpcDisconnection : public NetcodeRpcSession {
	Netcode::Network::PlayerDbDataRow playerData;

	void TerminateGameSession(int32_t userId) {
		serverSession->TerminateGameSession(userId, [this](const Netcode::Network::ErrorCode & ec) -> void {
			if(ec) {
				WrapSession(grpc::Status{ grpc::StatusCode::NOT_FOUND, "Game session was not found" });
			} else {
				auto * connData = response.mutable_connect_data();
				connData->set_nonce("tmp");
				connData->set_port(static_cast<int32_t>(serverSession->GetPort()));

				Ref<Netcode::Network::Connection> conn = serverSession->GetConnectionById(std::get<0>(playerData));
				serverSession->RemoveConnection(conn);
				
				WrapSession(grpc::Status::OK);
			}
		});
	}
	
	void AuthenticateUser(std::string hash) {
		serverSession->Authenticate(std::move(hash),
			[this](const Netcode::Network::ErrorCode & ec,
				const Netcode::Network::PlayerDbDataRow & ur) -> void {

			playerData = ur;

			response.set_type(request.type());

			if(ec) {
				WrapSession(grpc::Status{ grpc::StatusCode::NOT_FOUND, "Player session was not found" });
			} else {
				int32_t userId = std::get<0>(playerData);

				if(userId > 0) {
					TerminateGameSession(userId);
				} else {
					WrapSession(grpc::Status{ grpc::StatusCode::INTERNAL, "Query succeeded but got an invalid player ID" });
				}
			}

		});
	}
public:
	using NetcodeRpcSession::NetcodeRpcSession;

	virtual void Create() override {
		asyncService->RequestDisconnect(&context, &request, &responder, completionQueue, completionQueue, this);
		status = SessionStatus::PROCESS;
	}
	
	virtual void Process() override {
		(new NetcodeRpcDisconnection(asyncService, completionQueue, serverSession))->Run();

		auto mData = context.client_metadata();
		const auto it = mData.find("x-netcode-auth");

		if(it != mData.end()) {
			grpc::string_ref value = it->second;
			AuthenticateUser(std::string{ value.data(), value.size() });
		} else {
			WrapSession(grpc::Status{ grpc::StatusCode::INVALID_ARGUMENT, "Malformed request" });
		}
	}
};

class RpcServer final {
public:
	npss::AsyncService asyncService;
	std::unique_ptr<grpc::ServerCompletionQueue> completionQueue;
	std::unique_ptr<grpc::Server> rpcServer;
	std::unique_ptr<std::thread> workerThread;
	std::shared_ptr<Netcode::Network::ServerSession> ss;

	RpcServer() {
		grpc::EnableDefaultHealthCheckService(true);
	}

	void Start() {
		grpc::ServerBuilder builder;

		int port = 0;
		
		builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials(), &port);
		builder.RegisterService(&asyncService);
		completionQueue = builder.AddCompletionQueue();
		rpcServer = builder.BuildAndStart();

		workerThread = std::make_unique<std::thread>([this]() -> void {
			(new NetcodeRpcConnection(&asyncService, completionQueue.get(), ss))->Run();
			(new NetcodeRpcDisconnection(&asyncService, completionQueue.get(), ss))->Run();

			void * tag;
			bool ok;
			while(true) {
				completionQueue->Next(&tag, &ok);

				static_cast<RpcSessionBase *>(tag)->Run();
			}
		});
	}
	
};
