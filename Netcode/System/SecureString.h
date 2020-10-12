#pragma once

#include <Netcode/HandleDecl.h>
#include <NetcodeFoundation/ArrayView.hpp>
#include <string>

namespace Netcode {

	/*
	 * Dummy SecureString class to handle secrets a bit more eloquently
	 */
	class SecureString {
		uint8_t * encryptedData;
		mutable wchar_t * desc;
		uint32_t dataSizeInBytes;
		mutable uint32_t descSizeInChars;
		
		void ResetDesc() const;

		void ResetData();

	public:
		SecureString() noexcept;

		~SecureString() noexcept;

		SecureString(SecureString && rhs) noexcept;

		SecureString(const SecureString & rhs);

		SecureString & operator=(SecureString rhs);

		void Store(ArrayView<uint8_t> encryptedBinary);

		[[nodiscard]]
		ArrayView<uint8_t> GetView() const;
		
		void Store(std::wstring value, const std::wstring & description = L"");

		[[nodiscard]]
		std::wstring Load() const;

		[[nodiscard]]
		std::wstring_view GetDescription() const;

		[[nodiscard]]
		bool Empty() const;
	};
	
}
