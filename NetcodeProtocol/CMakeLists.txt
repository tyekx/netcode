cmake_minimum_required(VERSION 3.8)

netcode_add_static_library(NetcodeProtocol)

find_package(protobuf CONFIG REQUIRED)

target_link_libraries(NetcodeProtocol
PUBLIC
	protobuf::libprotobuf
)

set(PROTO_SRC shared.proto netcode.proto header.proto)

netcode_compile_proto(GENERATED_HEADERS GENERATED_SOURCE "${PROTO_SRC}")

target_sources(NetcodeProtocol
PUBLIC
	${GENERATED_HEADERS}

PRIVATE
	${GENERATED_SOURCE}
)

add_custom_target(NetcodeProtocolProtobuf DEPENDS ${GENERATED_FILES})

add_dependencies(NetcodeProtocol NetcodeProtocolProtobuf)
