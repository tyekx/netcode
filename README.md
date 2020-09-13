# Netcode

Implementing a multiplayer game for my thesis work. This project was mostly a way for me to learn DirectX 12 and graphics related questions. All while exploring the mysteries of a network protocol development.

# Build

Building is done through cmake. For the libraries, my recommendation is to use vcpkg. As of now, there are no ports for the `mysql/mysql-connector-cpp` library. However I created a crude workaround for this issue, for more information on getting the `mysql-connector-cpp` library, go to `Tools/vcpkg`.

# Libraries

Used 3rd party libraries:
 - NVIDIAGameWorks/PhysX (4.1)
 - Microsoft/DirectXTex (oct2019)
 - Microsoft/DirectXMath (?)
 - dropbox/json11 (1.0.0)
 - boostorg/boost (1.73.0)
 - protocolbuffers/protobuf (3.12.0)
 - grpc/grpc (1.31.1)
 - assimp/assimp (5.0.1)
 - mysql/mysql-connector-cpp (8.0.20)
