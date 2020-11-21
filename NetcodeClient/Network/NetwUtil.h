#pragma once

#include "../GameObject.h"
#include "../Scripts/RemotePlayerScript.h"

enum class HostMode : uint32_t {
	CLIENT, LISTEN, DEDICATED
};

struct Connection : public nn::ConnectionBase {
	RedundancyBuffer redundancyBuffer;
	GameObject * gameObject;
	RemotePlayerScript * remotePlayerScript;
	uint32_t localActionIndex;
	uint32_t remoteActionIndex;
	uint32_t localCommandIndex;
	uint32_t remoteCommandIndex;
	// cache members
	nn::GameMessage message;
	np::ServerUpdate * serverUpdate;


	Connection(boost::asio::io_context & ioc) : nn::ConnectionBase{ ioc },
		redundancyBuffer{}, gameObject{ nullptr }, remotePlayerScript{ nullptr },
		localActionIndex{ 1 }, remoteActionIndex{ 0 }, localCommandIndex{ 1 },
		remoteCommandIndex{ 0 }, message{}, serverUpdate{ nullptr } { }
};

struct ExtClientAction : public ClientAction {
	Connection * owner;
};

template<typename T>
class NodeIter {
	nn::Node<T> * current;

public:
	constexpr NodeIter(nn::Node<T> * msg) : current{ msg } { }

	constexpr nn::Node<T> * operator->() const {
		return current;
	}

	constexpr bool operator!=(std::nullptr_t) const {
		return current != nullptr;
	}

	NodeIter & operator++(int) {
		nn::Node<T> * tmp = current->next;
		current->allocator.reset();
		current = tmp;
		return *this;
	}
};

constexpr static uint32_t REPL_TYPE_REMOTE_AVATAR = 16;

struct ReplData {
	uint32_t objectId;
	std::string content;
};

std::string ReplicateWrite(GameObject * gameObject, Network * networkComponent);
void ReplicateRead(GameObject * gameObject, Network * networkComponent, const std::string & content, Netcode::GameClock* clock, int32_t connectionId, ActorType actor);

//TODO create a repldesc storage
Ref<ReplDesc> CreateRemoteAvatarReplDesc();
Ref<ReplDesc> ClientCreateRemoteAvatarReplDesc(RemotePlayerScript * rps);
Ref<ReplDesc> CreateLocalAvatarReplDesc(Camera * cameraComponent);

np::ClientUpdate * ParseClientUpdate(nn::GameMessage * message);

Netcode::Float3 ConvertFloat3(const np::Float3 & f3);

void ConvertFloat3(np::Float3 * dst, const Netcode::Float3 & src);

GameObject * CreateRemoteAvatar(Netcode::Duration interpDelay);
