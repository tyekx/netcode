#pragma once

#include <NetcodeFoundation/Exceptions.h>
#include "File.h"


namespace Netcode::IO {

	class BinaryReader
	{
		uint8_t * pos;
		uint8_t * end;

		std::unique_ptr<uint8_t[]> ownedData;
	public:
		BinaryReader(File & f);
		BinaryReader(std::unique_ptr<uint8_t[]> dataBlob, size_t dataSize);

		BinaryReader(BinaryReader const &) = delete;
		BinaryReader & operator=(BinaryReader const &) = delete;

		template<typename T>
		const T & Read()
		{
			return ReadArray<T>(1)[0];
		}

		template<typename T>
		ArrayView<T> ReadArray(size_t elementCount)
		{
			static_assert(std::is_pod<T>::value, "Can only read plain-old-data types");

			uint8_t * newPos = pos + sizeof(T) * elementCount;

			if(newPos < pos) {
				Netcode::OutOfRangeAssertion("Out of range");
			}

			if(newPos > end) {
				Netcode::OutOfRangeAssertion("End of file");
			}

			auto result = reinterpret_cast<T *>(pos);

			pos = newPos;

			return ArrayView<T>{ result, elementCount };
		}

	};

}
