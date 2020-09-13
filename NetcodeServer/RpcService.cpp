#include "RpcService.h"

::grpc::Status RpcServiceImpl::Connect(::grpc::ServerContext * context, const::Netcode::Protocol::Request * request, ::Netcode::Protocol::Response * response)
{
	response->set_type(request->type());
	return ::grpc::Status::OK;
}
