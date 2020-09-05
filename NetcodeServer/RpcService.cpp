#include "RpcService.h"

::grpc::Status RpcServiceImpl::Connect(::grpc::ServerContext * context, const::Netcode::Protocol::Control::Request * request, ::Netcode::Protocol::Control::Response * response)
{
	return ::grpc::Status::OK;
}
