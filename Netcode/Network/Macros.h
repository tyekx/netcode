#pragma once

#define RETURN_IF(condition) if(condition) return

#define RETURN_IF_AND_LOG_ERROR(condition, msg) if(condition) { \
	Log::Error(msg); \
	return;	\
}

#define RETURN_ON_ERROR(ec, msg)	\
if(ec) {	\
	Log::Error(msg, ec.message()); \
	return;	\
}

#define RETURN_VALUE_ON_ERROR(ec, msg, rv)	\
if(ec) {	\
	Log::Error(msg, ec.message()); \
	return rv;	\
}

#define RETURN_AND_LOG_IF_ABORTED(ec, msg) \
if(ec == boost::asio::error::operation_aborted) {  \
	Log::Info( msg ); \
	return;  \
}
