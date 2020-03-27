#pragma once

#ifndef STREAM_MESSAGE_H
#define STREAM_MESSAGE_H

#include "Endpoint.h"

struct StreamMessage {
	std::string Data;
	std::size_t Bytes;
	SharedEndpoint Subject;

	StreamMessage();
};

using SharedStreamMessage = std::shared_ptr<StreamMessage>;

#endif
