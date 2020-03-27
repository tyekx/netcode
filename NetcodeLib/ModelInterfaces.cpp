#include "ModelInterfaces.h"

std::ostream & operator<<(std::ostream & os, IField & field) {
	field.Print(os);
	return os;
}

