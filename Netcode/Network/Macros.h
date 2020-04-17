#pragma once

#define RETURN_IF(condition) if(condition) return

#define RETURN_IF_AND_LOG_ERROR(condition, msg) if(condition) { \
	Log::Error(msg); \
	lastError = boost::asio::error::make_error_code(boost::asio::error::fault); \
	return;	\
}

#define RETURN_ON_ERROR(ec, msg)	\
if(ec) {	\
	Log::Error(msg, ec.message()); \
	lastError = ec;	\
	return;	\
}

#define RETURN_VALUE_ON_ERROR(ec, msg, rv)	\
if(ec) {	\
	Log::Error(msg, ec.message()); \
	lastError = ec;	\
	return rv;	\
}

#define RETURN_AND_LOG_IF_ABORTED(ec, msg) \
if(ec == boost::asio::error::operation_aborted) {  \
	Log::Info( msg ); \
	return;  \
}
