#pragma once

#include <NetcodeProtocol/netcode.grpc.pb.h>

class RpcServiceImpl : public Netcode::Protocol::ServerService::Service
{
public:
	::grpc::Status Connect(::grpc::ServerContext * context, const ::Netcode::Protocol::Control::Request * request,
		::Netcode::Protocol::Control::Response * response) override;
};
