#include "Json.h"
#include "File.h"

namespace Netcode::IO {

	void ParseJson(rapidjson::Document & doc, std::wstring_view path) {
		File jsonFile{ std::wstring{ path } };
		FileReader<File> reader{ jsonFile, FileOpenMode::READ };

		size_t numBytes = reader->GetSize();
		std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(numBytes);
		MutableArrayView<uint8_t> view{ data.get(), numBytes };

		reader->Read(view);

		rapidjson::MemoryStream stream{ reinterpret_cast<char *>(view.Data()), view.Size() };
		doc.ParseStream(stream);
	}

}
