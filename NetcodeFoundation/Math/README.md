# Netcode Math

FloatN is storage only
VectorX, Matrix, Quaternion is for computation

VectorX::operator+,-,*,/, are for elementwise multiplication
since Matrix, Quaternion types are considered to be higher level, operator+,-,/ is not defined for these types, but operator* is their proper multiplication operand
