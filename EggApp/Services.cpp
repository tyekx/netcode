#include "Services.h"

uint8_t Service::data[TupleSizeofSum<ServicesTuple>::value] = {};
SignatureType Service::signature = 0;
