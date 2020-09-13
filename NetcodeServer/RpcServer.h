#pragma once

#include <memory>
#include <NetcodeProtocol/netcode.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <Netcode/Network/ServerSession.h>

using npss = Netcode::Protocol::ServerService;
namespace np = Netcode::Protocol;

class RpcServerSession {
	
	enum class SessionStatus : uint32_t { CREATE, PROCESS, FINISH };
	
	npss::AsyncService * asyncService;
	grpc::ServerCompletionQueue * completionQueue;
	grpc::ServerContext context;
	grpc::ServerAsyncResponseWriter<np::Response> responder;
	np::Request request;
	np::Response response;
	SessionStatus status;
	Netcode::Network::PlayerDbDataRow playerRow;
	Netcode::Network::ErrorCode errorCode;
public:
	Ref<Netcode::Network::ServerSession> sess;
	
	RpcServerSession(npss::AsyncService * asyncService, grpc::ServerCompletionQueue * cq, Ref<Netcode::Network::ServerSession> ss) :
		asyncService{ asyncService }, completionQueue{ cq }, context{}, responder{ &context },
		request{}, response{}, status{ SessionStatus::CREATE }, playerRow{}, errorCode{}, sess{ ss } {
		Proceed();
	}

	void Proceed() {
		if(status == SessionStatus::CREATE) {
			status = SessionStatus::PROCESS;
			std::cout << this << "CREATE" << std::endl;
			asyncService->RequestConnect(&context, &request, &responder, completionQueue, completionQueue, this);
			return;
		}

		if(status == SessionStatus::PROCESS) {
			new RpcServerSession(asyncService, completionQueue, sess);

			auto mData = context.client_metadata();
			const auto it = mData.find("x-netcode-auth");

			if(it != mData.end()) {
				grpc::string_ref value = it->second;
				// start db query
				std::cout << this << "PROCESS" << std::endl;
				sess->Authenticate(std::string{ value.data(), value.size() }, [this](const Netcode::Network::ErrorCode & ec, const Netcode::Network::PlayerDbDataRow & ur) -> void {
					playerRow = ur;
					errorCode = ec;

					response.set_type(request.type());
					std::cout << this << "DB_RESULT" << std::endl;

					if(errorCode) {
						std::cout << "Failed to get user: " << errorCode.message() << std::endl;
						response.set_error_code(1);
					} else {
						std::cout << "Auth ok: " << std::get<1>(playerRow) << std::endl;
						response.mutable_connect_data()->set_port(8889);
						response.set_error_code(0);
					}

					responder.Finish(response, grpc::Status::OK, this);
					status = SessionStatus::FINISH;
				});
			} else {
				status = SessionStatus::FINISH;
				response.set_error_code(1);
				responder.Finish(response, grpc::Status::OK, this);
			}
		}

		if(status == SessionStatus::FINISH) {
			std::cout << this << "FINISH" << std::endl;
			delete this;
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
			new RpcServerSession(&asyncService, completionQueue.get(), ss);

			void * tag;
			bool ok;
			while(true) {
				completionQueue->Next(&tag, &ok);

				static_cast<RpcServerSession *>(tag)->Proceed();
			}
		});
	}
	
};
