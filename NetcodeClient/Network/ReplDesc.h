#pragma once

#include <NetcodeFoundation/ArrayView.hpp>
#include <Netcode/Network/NetworkCommon.h>

class GameObject;

enum class ActorType {
	CLIENT, SERVER
};

enum class ReplType {
	/**
	 * Default type will be replicated the same way on all actors
	 */
	DEFAULT = 0x1,
	
	/**
	 * The server will reject any updates to that value
	 * and only accepts them through other means like actions
	 */
	CLIENT_PREDICTED = 0x2
};

class ReplArgumentBase {
protected:
	ReplType type;

public:
	virtual ~ReplArgumentBase() = default;
	ReplArgumentBase(ReplType type) : type { type } { }
	NETCODE_CONSTRUCTORS_DEFAULT_MOVE(ReplArgumentBase);
	NETCODE_CONSTRUCTORS_DELETE_COPY(ReplArgumentBase);
	
	ReplType GetType() const { return type; }
	
	/**
	 * @param gameObject the serialization subject for stateless operation
	 * @return the number of bytes required to replicate this value
	 */
	virtual uint32_t GetReplicatedSize(GameObject * gameObject) const = 0;

	/**
	 * @return the number of bytes it can consume from the buffer
	 */
	virtual uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> view) const = 0;

	/**
	 * Serializes the argument into the given buffer
	 * @param gameObject the serialization subject for stateless operation
	 * @return the number of bytes written, 0 means failure
	 */
	virtual uint32_t Write(GameObject * gameObject, Netcode::MutableArrayView<uint8_t> view) const = 0;

	/**
	 * Deserializes the argument from the buffer
	 * @param gameObject the serialization subject for stateless operation
	 * @return the number of bytes read from the buffer, 0 means failure
	 */
	virtual uint32_t Read(GameObject * gameObject, Netcode::ArrayView<uint8_t> view) = 0;
};

using ReplDesc = std::vector<std::unique_ptr<ReplArgumentBase>>;
