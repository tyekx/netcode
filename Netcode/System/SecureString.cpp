#include "SecureString.h"
#include <NetcodeFoundation/Platform.h>

#if defined(NETCODE_OS_WINDOWS)
#include <Windows.h>
#include <dpapi.h>
#pragma comment(lib, "Crypt32.lib")
#endif

namespace Netcode {

	void SecureString::ResetDesc() const {
		if(desc != nullptr) {
			LocalFree(desc);
			desc = nullptr;
			descSizeInChars = 0;
		}
	}

	void SecureString::ResetData() {
		if(encryptedData != nullptr) {
			LocalFree(encryptedData);
			encryptedData = nullptr;
			dataSizeInBytes = 0;
		}
	}

	SecureString::SecureString() noexcept : encryptedData{ nullptr }, desc{ nullptr }, dataSizeInBytes{ 0 }, descSizeInChars{ 0 } {
		
	}

	SecureString::~SecureString() noexcept {
		ResetDesc();
		ResetData();
	}

	SecureString::SecureString(SecureString && rhs) noexcept : SecureString{} {
		std::swap(encryptedData, rhs.encryptedData);
		std::swap(desc, rhs.desc);
		std::swap(dataSizeInBytes, rhs.dataSizeInBytes);
		std::swap(descSizeInChars, rhs.descSizeInChars);
	}

	SecureString::SecureString(const SecureString & rhs) : SecureString{} {
		if(rhs.encryptedData == nullptr) {
			return;
		}

		encryptedData = reinterpret_cast<uint8_t *>(LocalAlloc(0, rhs.dataSizeInBytes));
		memcpy(encryptedData, rhs.encryptedData, rhs.dataSizeInBytes);
		dataSizeInBytes = rhs.dataSizeInBytes;
	}

	SecureString & SecureString::operator=(SecureString rhs) {
		std::swap(encryptedData, rhs.encryptedData);
		std::swap(desc, rhs.desc);
		std::swap(dataSizeInBytes, rhs.dataSizeInBytes);
		std::swap(descSizeInChars, rhs.descSizeInChars);
		return *this;
	}

	void SecureString::Store(ArrayView<uint8_t> encryptedBinary) {
		// either we are the owners, or both is null
		if(encryptedData == encryptedBinary.Data()) {
			return;
		}

		ResetData();
		ResetDesc();

		if(encryptedBinary.Data() != nullptr && encryptedBinary.Size() != 0) {
			encryptedData = reinterpret_cast<BYTE *>(LocalAlloc(0, encryptedBinary.Size()));
			dataSizeInBytes = static_cast<uint32_t>(encryptedBinary.Size());
			memcpy(encryptedData, encryptedBinary.Data(), encryptedBinary.Size());
		}
	}

	ArrayView<uint8_t> SecureString::GetView() const {
		return ArrayView<uint8_t>{ encryptedData, dataSizeInBytes };
	}

	void SecureString::Store(std::wstring value, const std::wstring & description) {
		ResetData();
		ResetDesc();
		const wchar_t * tmpDesc = (description.empty()) ? nullptr : description.data();
		DATA_BLOB inData;
		inData.pbData = reinterpret_cast<BYTE *>(value.data());
		inData.cbData = value.size() * sizeof(std::wstring::value_type);

		DATA_BLOB outData;
		outData.pbData = nullptr;
		outData.cbData = 0;
		
		if(!CryptProtectData(&inData, tmpDesc, nullptr, nullptr, nullptr, 0, &outData)) {
			return;
		}

		encryptedData = outData.pbData;
		dataSizeInBytes = outData.cbData;
	}

	std::wstring SecureString::Load() const {
		std::wstring wstr;
		DATA_BLOB inData;
		inData.pbData = encryptedData;
		inData.cbData = dataSizeInBytes;
		DATA_BLOB outData;
		outData.pbData = nullptr;
		outData.cbData = 0;
		ResetDesc();
		if(!CryptUnprotectData(&inData, &desc, nullptr, nullptr, nullptr, 0, &outData)) {
			return std::wstring{ };
		}
		descSizeInChars = static_cast<uint32_t>(wcslen(desc));
		wstr = std::wstring_view{ reinterpret_cast<wchar_t *>(outData.pbData), outData.cbData / sizeof(std::wstring::value_type) };
		LocalFree(outData.pbData);
		return wstr;
	}

	std::wstring_view SecureString::GetDescription() const
	{
		return std::wstring_view{ desc, descSizeInChars };
	}

	bool SecureString::Empty() const
	{
		return encryptedData == nullptr;
	}

}
