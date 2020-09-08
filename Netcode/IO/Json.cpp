#include <NetcodeFoundation/Exceptions.h>
#include <Netcode/Logger.h>
#include <Netcode/Utility.h>
#include "Json.h"
#include "File.h"

namespace Netcode::IO {

	void ParseJsonFromFile(JsonDocument & doc, const std::wstring & path) {
		File jsonFile{ path };

		try {
			FileReader<File> reader{ jsonFile };
			size_t numBytes = reader->GetSize();
			std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(numBytes);
			MutableArrayView<uint8_t> view{ data.get(), numBytes };
			reader->Read(view);

			if(!Utility::CheckBinaryDataForUTF16LE(view)) {
				JsonSerializer<JsonDocument>::Load<rapidjson::UTF8<>>(doc, view);
			} else {
				JsonSerializer<JsonDocument>::Load<>(doc, view);
			}
			
		} catch(ExceptionBase& ex) {
			Log::Error("While parsing JSON: {0}", ex.ToString());
		}
	}

}
