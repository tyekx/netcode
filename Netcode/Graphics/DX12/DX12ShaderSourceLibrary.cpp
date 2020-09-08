#include "DX12ShaderSourceLibrary.h"

#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>

namespace Netcode::Graphics::DX12 {

	const std::string & ShaderSourceLibrary::GetSource(const std::wstring & key) {

		decltype(lib)::const_iterator it = lib.find(key);

		if(it == lib.end()) {
			IO::File shaderFile{ IO::Path::ShaderRoot(), key };
			IO::FileReader<IO::File> reader{ shaderFile };

			size_t numBytes = reader->GetSize();

			std::string dst;
			dst.resize(numBytes);

			reader->Read(Netcode::MutableArrayView<uint8_t>{ reinterpret_cast<uint8_t *>(dst.data()), numBytes });

			lib[key] = std::move(dst);

			it = lib.find(key);
		}

		return it->second;
	}

}
