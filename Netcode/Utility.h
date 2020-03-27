#pragma once

#include "Common.h"
#include <string>

namespace Netcode {

	namespace Utility {

		/*
		Checkes for an unsigned int if its a power of 2. Wasteful approach but working for edge cases including 0.
		*/
		constexpr bool IsPowerOf2(unsigned int v) {
			unsigned int bitCount = 0;
			for(; v > 0; v >>= 0x1) {
				if(v & 0x1) {
					++bitCount;
				}
			}
			return bitCount == 1;
		}


		/*
		Takes a number and returns its closest multiplicative of 65536. The returned number is always bigger or equal than the input number
		*/
		template<typename INT_T = uint32_t>
		constexpr INT_T Align64K(INT_T v) {
			return (v + INT_T{ 0xFFFF }) & (~INT_T{ 0xFFFF });
		}

		/*
		Takes a number and returns its closest multiplicative of 256. The returned number is always bigger or equal than the input number
		*/
		template<typename INT_T = uint32_t>
		constexpr INT_T Align256(INT_T v) {
			return (v + INT_T{ 255 }) & (~INT_T{ 255 });
		}

		template<typename INT_T = int>
		constexpr INT_T Align512(INT_T v) {
			return (v + INT_T{ 511 }) & (~(INT_T{ 511 }));
		}

		/*
		formats a wide string into a std::wstring
		*/
		std::wstring WFormat(const wchar_t * format, ...);

		/*
		Prints a widechar printf-like message to the Visual Studio Output window
		*/
		void WDebugf(const wchar_t * format, ...);

		/*
		Prints a printf-like message to the Visual Studio Output window
		*/
		void Debugf(const char * format, ...);

		/*
		Loads a file into a string
		*/
		bool SlurpFile(std::string & dstBuffer, const std::wstring & filepath);

		std::string ToNarrowString(const std::wstring & wideString);

		std::wstring ToWideString(const std::string & narrowString);

		/*
std::vector<std::string_view> Split(std::string_view str, std::string_view delims = " ")
{
	std::vector<std::string_view> output;

	for(auto first = str.data(), second = str.data(), last = first + str.size(); second != last && first != last; first = second + 1) {
		second = std::find_first_of(first, last, std::cbegin(delims), std::cend(delims));

		if(first != second)
			output.emplace_back(first, second - first);
	}

	return output;
}*/

		/*
		prints the WinAPI event to VS output window, use the filter argument to not print those messages
		*/
		void DebugEvent(unsigned int msg, std::initializer_list<unsigned int> filter = {});



	}

}
