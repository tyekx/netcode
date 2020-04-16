cmake_minimum_required(VERSION 3.8)

add_library(NetcodeProtocol STATIC "")

target_include_directories(NetcodeProtocol SYSTEM
PUBLIC
	"${PROTOBUF_INCLUDE}"
)

target_link_directories(NetcodeProtocol
PUBLIC
	"${PROTOBUF_LIB}"
)

target_link_libraries(NetcodeProtocol "${PROTOBUF_LINK}")

set(PROTO_SRC
	client.proto
	control.proto
	netcode.proto
	server.proto
	shared.proto
)

netcode_compile_proto(GENERATED_FILES "${PROTO_SRC}")

target_sources(NetcodeProtocol PUBLIC ${GENERATED_FILES})

add_custom_target(NetcodeProtocolProtobuf DEPENDS ${GENERATED_FILES})

add_dependencies(NetcodeProtocol NetcodeProtocolProtobuf)
