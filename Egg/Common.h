#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include "d3dx12.h"
#include <memory>
#include <vector>
#include <dxgi1_6.h>
#include <wrl.h>

template<typename T>
using com_ptr = Microsoft::WRL::ComPtr<T>;

namespace Egg {

	namespace Internal {

		void Assert(bool trueMeansOk, const char * msgOnFail, ...);

		class HResultTester {
		public:
			const char * message;
			const char * file;
			const int line;
			va_list args;

			HResultTester(const char * msg, const char * file, int line, ...);

			void operator<<(HRESULT hr);
		};

	}

}


// the message is assumed to be a printf format string, the variadic arguments assumed to be the arguments for that format
#define ASSERT(trueMeansOk, msgOnFail, ...) Egg::Internal::Assert(trueMeansOk, msgOnFail, __VA_ARGS__)

#define DX_API(msg, ...) Egg::Internal::HResultTester(msg, __FILE__, __LINE__, __VA_ARGS__) <<

