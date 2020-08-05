#pragma once

namespace boost::system {

	class error_code;

}

namespace boost::asio {

	class io_context;
	class executor;

	namespace ip {

		class udp;
		class tcp;

	}

}

namespace Netcode::Network {

	class Timer;
	using ErrorCode = boost::system::error_code;

}
