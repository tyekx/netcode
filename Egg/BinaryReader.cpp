#include "BinaryReader.h"
#include <fstream>
#include <algorithm>

namespace Egg {
	// Constructor reads from the filesystem.
	BinaryReader::BinaryReader(_In_z_ wchar_t const * fileName) :
		mPos(nullptr),
		mEnd(nullptr)
	{
		size_t dataSize;

		bool hr = ReadEntireFile(fileName, mOwnedData, &dataSize);
		ASSERT(hr, "Failed to read file");

		mPos = mOwnedData.get();
		mEnd = mOwnedData.get() + dataSize;
	}


	// Constructor reads from an existing memory buffer.
	BinaryReader::BinaryReader(_In_reads_bytes_(dataSize) uint8_t * dataBlob, size_t dataSize) :
		mPos(dataBlob),
		mEnd(dataBlob + dataSize)
	{
	}


	// Reads from the filesystem into memory.
	bool BinaryReader::ReadEntireFile(_In_z_ wchar_t const * fileName, _Inout_ std::unique_ptr<uint8_t[]> & data, _Out_ size_t * dataSize)
	{
		std::ifstream ifs{ fileName, std::ios::binary };

		if(!ifs.is_open()) {
			return false;
		}

		ifs.seekg(0, std::ios::end);
		*dataSize = ifs.tellg();

		data = std::make_unique<uint8_t[]>(*dataSize);

		ifs.seekg(0, std::ios::beg);

		ifs.read(reinterpret_cast<char *>(data.get()), *dataSize);

		return true;
	}


}
