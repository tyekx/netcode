#pragma once

#include <NetcodeProtocol/netcode.grpc.pb.h>

class RpcServiceImpl : public Netcode::Protocol::ServerService::AsyncService
{
public:
	::grpc::Status Connect(::grpc::ServerContext * context, const ::Netcode::Protocol::Request * request,
		::Netcode::Protocol::Response * response) override;
};
