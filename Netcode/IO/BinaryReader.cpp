#include "BinaryReader.h"

namespace Netcode::IO {



    BinaryReader::BinaryReader(File & f) : pos{ nullptr }, end{ nullptr }, ownedData{ nullptr }
    {
        FileReader<File> reader{ f, FileOpenMode::READ };
        size_t numBytes = reader->GetSize();
        ownedData = std::make_unique<uint8_t[]>(numBytes);
        reader->Read(MutableArrayView<uint8_t>{ ownedData.get(), numBytes });
        pos = ownedData.get();
        end = pos + numBytes;
    }

    BinaryReader::BinaryReader(std::unique_ptr<uint8_t[]> dataBlob, size_t dataSize) : pos{ dataBlob.get() }, end{ dataBlob.get() + dataSize }, ownedData{ std::move(dataBlob) }
    {

    }

}
