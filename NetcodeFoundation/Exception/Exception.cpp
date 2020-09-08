#include "IOException.h"
#include "Common.h"
#include <sstream>

namespace Netcode {
	
	std::string ExceptionBase::ToString() const {
		std::ostringstream oss;
		oss << what_type() << ": " << message;
		return oss.str();
	}

	std::string IOException::ToString() const {
		std::ostringstream oss;
		oss << what_type() << ": " << message << ". Argument: '" << filePath << "'";
		return oss.str();
	}

}
