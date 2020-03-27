#include "Services.h"

template<>
uint8_t Service::data[TupleSizeofSum<ServicesTuple>::value] = {};

template<>
SignatureType Service::signature = 0;
