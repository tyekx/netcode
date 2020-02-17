#include "MemoryFunctions.h"

namespace Egg {

	void InterpretSkip(void ** ptr, unsigned int nBytes) {
		*ptr = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(*ptr) + nBytes);
	}

}
