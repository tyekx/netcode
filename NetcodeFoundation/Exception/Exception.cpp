#include "IOException.h"
#include "Common.h"
#include <sstream>

namespace Netcode {
	
	std::string ExceptionBase::ToString() const {
		std::ostringstream oss;
		oss << what_type() << ": " << what();
		return oss.str();
	}

	std::string IOException::ToString() const {
		std::ostringstream oss;
		oss << what_type() << ": " << what() << ". Argument: '" << filePath << "'";
		return oss.str();
	}

}
