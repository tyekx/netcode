#include "DX12Common.h"
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include "Utility.h"

namespace Egg::Graphics::DX12 {
	
	void DebugPrintBlob(com_ptr<ID3DBlob> blob) {
		if(blob != nullptr) {
			OutputDebugString(reinterpret_cast<const char *>(blob->GetBufferPointer()));
		} else {
			OutputDebugString("Blob was NULL");
		}
	}

	void GetAdapters(IDXGIFactory6 * dxgiFactory, std::vector<com_ptr<IDXGIAdapter1>> & adapters) {
		com_ptr<IDXGIAdapter1> tempAdapter{ nullptr };
		OutputDebugStringW(L"Detected Video Adapters:\n");
		unsigned int adapterId = 0;

		for(HRESULT query = dxgiFactory->EnumAdapters1(adapterId, tempAdapter.GetAddressOf());
			query != DXGI_ERROR_NOT_FOUND;
			query = dxgiFactory->EnumAdapters1(++adapterId, tempAdapter.GetAddressOf())) {

			// check if not S_OK
			DX_API("Failed to query DXGI adapter") query;

			if(tempAdapter != nullptr) {
				DXGI_ADAPTER_DESC desc;
				tempAdapter->GetDesc(&desc);

				Egg::Utility::WDebugf(L"    %s\n", desc.Description);

				adapters.push_back(std::move(tempAdapter));
				tempAdapter.Reset();
			}
		}
	}

	const char * FeatureLevelToString(D3D_FEATURE_LEVEL ftlvl) {
		switch(ftlvl) {
		case D3D_FEATURE_LEVEL_9_1: return "Feature level 9.1";
		case D3D_FEATURE_LEVEL_9_2: return "Feature level 9.2";
		case D3D_FEATURE_LEVEL_9_3: return "Feature level 9.3";
		case D3D_FEATURE_LEVEL_10_0: return "Feature level 10.0";
		case D3D_FEATURE_LEVEL_10_1: return "Feature level 10.1";
		case D3D_FEATURE_LEVEL_11_0: return "Feature level 11.0";
		case D3D_FEATURE_LEVEL_11_1: return "Feature level 11.1";
		case D3D_FEATURE_LEVEL_12_0: return "Feature level 12.0";
		case D3D_FEATURE_LEVEL_12_1: return "Feature level 12.1";
		default: return "Unknown feature level";
		}
	}

}

Egg::Internal::HResultTester::HResultTester(const char * msg, const char * file, int line, ...) :
	message{ msg }, file{ file }, line{ line } {
	va_list l;
	va_start(l, line);
	va_copy(args, l);
	va_end(l);

}

void Egg::Internal::HResultTester::operator<<(HRESULT hr) {
	if(FAILED(hr)) {
		std::ostringstream oss;
		oss << file << "(" << line << "): " << message;
		std::string buffer;
		buffer.resize(1024);

		vsprintf_s(&(buffer.at(0)), 1024, oss.str().c_str(), args);
		va_end(args);

		OutputDebugString(buffer.c_str());
		OutputDebugString("\r\n");
		if(IsDebuggerPresent()) {
			DebugBreak();
		}
		exit(-1);
	}
	va_end(args);
}
