#include "../../Utility.h"
#include "DX12Common.h"
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <comdef.h>

namespace Netcode::Graphics::DX12 {
	
	void DebugPrintBlob(com_ptr<ID3DBlob> blob) {
		if(blob != nullptr) {
			OutputDebugString(reinterpret_cast<const char *>(blob->GetBufferPointer()));
		} else {
			OutputDebugString("Blob was NULL");
		}
	}

	std::wstring RootSigDebugName(const std::wstring & filename)
	{
		std::wostringstream woss;
		woss << L"RS(";
		size_t indexOf = filename.find_last_of(L'\\');
		if(indexOf == std::wstring::npos) {
			indexOf = filename.find_last_of(L'/');
		}

		if(indexOf == std::wstring::npos) {
			woss << filename << L')';
		} else {
			woss << filename.substr(indexOf + 1) << L')';
		}
		return woss.str();
	}

	std::wstring IndexedDebugName(const char * prefix, uint32_t idx)
	{
		std::wostringstream woss;
		woss << prefix << L'(' << idx << L')';
		return woss.str();
	}

	const char * RootSignatureVersionToString(D3D_ROOT_SIGNATURE_VERSION version)
	{
		switch(version) {
			case D3D_ROOT_SIGNATURE_VERSION_1_0: return "Root Signature Version 1.0";
			case D3D_ROOT_SIGNATURE_VERSION_1_1: return "Root Signature Version 1.1";
			default: return "Unknown Root Signature Version";
		}
	}

	const char * ShaderModelToString(D3D_SHADER_MODEL shaderModel) {
		switch(shaderModel) {
			case D3D_SHADER_MODEL_5_1: return "Shader Model 5.1";
			case D3D_SHADER_MODEL_6_0: return "Shader Model 6.0";
			case D3D_SHADER_MODEL_6_1: return "Shader Model 6.1";
			case D3D_SHADER_MODEL_6_2: return "Shader Model 6.2";
			case D3D_SHADER_MODEL_6_3: return "Shader Model 6.3";
			case D3D_SHADER_MODEL_6_4: return "Shader Model 6.4";
			default: return "Unknown shader model";
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

	const char * CommandListTypeToString(D3D12_COMMAND_LIST_TYPE type) {
		switch(type) {
			case D3D12_COMMAND_LIST_TYPE_DIRECT: return "Direct Command List";
			case D3D12_COMMAND_LIST_TYPE_BUNDLE: return "Bundle Command List";
			case D3D12_COMMAND_LIST_TYPE_COMPUTE: return "Compute Command List";
			case D3D12_COMMAND_LIST_TYPE_COPY: return "Copy Command List";
			case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE: return "Video Decode Command List";
			case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS: return "Video Process Command List";
			default: return "Unknown Command List";
		}
	}

	const char * GetBlobDataAsString(com_ptr<ID3DBlob> blob)
	{
		if(blob != nullptr) {
			return static_cast<const char *>(blob->GetBufferPointer());
		} else {
			return "Blob was null";
		}
	}

}

Netcode::Internal::HResultTester::HResultTester(const char * msg, const char * file, int line, ...) :
	message{ msg }, file{ file }, line{ line } {
	va_list l;
	va_start(l, line);
	va_copy(args, l);
	va_end(l);
}

void Netcode::Internal::HResultTester::operator<<(HRESULT hr) {
	if(FAILED(hr)) {
		std::ostringstream oss;
		oss << file << "(" << line << "): " << message;
		std::string buffer;
		buffer.resize(1024);

		vsprintf_s(&(buffer.at(0)), 1024, oss.str().c_str(), args);
		va_end(args);

		Log::Error(buffer.c_str());

		_com_error err(hr);
		Log::Info(err.ErrorMessage());

		if(IsDebuggerPresent()) {
			DebugBreak();
		}
		exit(-1);
	}
	va_end(args);
}
