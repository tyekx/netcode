#pragma once

#include "Common.h"
#include <string>

namespace Egg {

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
		constexpr unsigned int Align64K(unsigned int v) {
			return (v + 0xFFFF) & (~0xFFFF);
		}

		/*
		Takes a number and returns its closest multiplicative of 256. The returned number is always bigger or equal than the input number
		*/
		constexpr unsigned int Align256(unsigned int v) {
			return (v + 255U) & (~255U);
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
		prints the WinAPI event to VS output window, use the filter argument to not print those messages
		*/
		void DebugEvent(unsigned int msg, std::initializer_list<unsigned int> filter = {});



	}

}
