# Netcode

Implementing a multiplayer game for my thesis work. This project was mostly a way for me to learn DirectX 12 and graphics related questions. All while exploring the mysteries of a network protocol development.

_Note: I used this project to try many different coding styles, C++ techniques and idioms, there will be some inconsistencies throughout the repository_

# Folders

- `Media`: assets for the game
- `Netcode`: shared functionality between built executables. Thin abstraction layer on the DirectX 12 API. Uses different memory pools and tries to save memory by using Placed Resources. Can render spritefonts, using an altered version of the spritefont renderer found in the DXTK library. Handles Rendering, UI, Animation, Network, Input, and File IO.
- `NetcodeAssetCompiler`: Command line tool to build assets
- `NetcodeAssetEditor`: Visual asset editor, allows the user to import FBX files. Used to attach colliders and merge animations into a single, load-optimized binary file format. Also allows the user to save a manifest file, that creates a dependency between FBX files. These manifests files can be used with the NetcodeAssetCompiler to automatize rebuilding of assets. Written in C++/WinRT to explore WinRT with XAML.
- `NetcodeAssetLib`: shared functionality between `NetcodeAssetEditor` and `NetcodeAssetCompiler`
- `NetcodeClient`: The main program that contains the game logic. Currently only client and client-hosted-server (aka. listen server) modes are supported. Currently it is pretty hard-coded, did not invest time into creating a scriptable API, so the "scripts" are all in C++. Follows an ECS architecture. The rendering is done through a render graph, which is based on Yuriy O'Donnell GDC talk about Frostbite. Through that I tried a lot of different rendering techniques, even swapped between forward and deferred renderer. Very versatile mode to render. For the code see [GraphicsEngine.cpp](NetcodeClient/GraphicsEngine.cpp), [DX12FrameGraphExecutor.cpp](Netcode/Graphics/DX12/DX12FrameGraphExecutor.cpp).
- `NetcodeFoundation`: most common functions, math library, error handling, allocator attempts.
- `NetcodeProtocol`: protobuf project that compiles into a library which can be linked to other projects
- `NetcodeServer`: proof of concept folder for exploring options, not important, already merged into `NetcodeClient`
- `NetcodeUnitTests`: As the name implies
- `NetcodeWeb*`: the website backend, frontend and database schema
- `Tools`: utility

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
