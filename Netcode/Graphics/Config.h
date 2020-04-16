#include <cstdint>
#include <string>

namespace Netcode::Graphics {

	struct Config {
		uint32_t width;
		uint32_t height;
		uint8_t graphicsAdapterIndex;
		uint8_t displayIndex;
		uint8_t backbufferDepth;
		bool fullscreen;
	};

}
