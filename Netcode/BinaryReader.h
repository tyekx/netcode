#pragma once

#include "Common.h"
#include <exception>
#include <stdexcept>

namespace Netcode {
	class BinaryReader
	{
	public:
		explicit BinaryReader(_In_z_ wchar_t const * fileName);
		BinaryReader(_In_reads_bytes_(dataSize) uint8_t * dataBlob, size_t dataSize);

		BinaryReader(BinaryReader const &) = delete;
		BinaryReader & operator= (BinaryReader const &) = delete;

		// Reads a single value.
		template<typename T> const T & Read()
		{
			return *ReadArray<T>(1);
		}


		// Reads an array of values.
		template<typename T> T * ReadArray(size_t elementCount)
		{
			static_assert(std::is_pod<T>::value, "Can only read plain-old-data types");

			uint8_t * newPos = mPos + sizeof(T) * elementCount;

			if(newPos < mPos)
				throw std::overflow_error("ReadArray");

			if(newPos > mEnd)
				throw std::exception("End of file");

			auto result = reinterpret_cast<T *>(mPos);

			mPos = newPos;

			return result;
		}


		// Lower level helper reads directly from the filesystem into memory.
		static bool ReadEntireFile(_In_z_ wchar_t const * fileName, _Inout_ std::unique_ptr<uint8_t[]> & data, _Out_ size_t * dataSize);


	private:
		// The data currently being read.
		uint8_t * mPos;
		uint8_t * mEnd;

		std::unique_ptr<uint8_t[]> mOwnedData;
	};

}
