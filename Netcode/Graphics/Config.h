#pragma once

#include <rapidjson/document.h>

#include <NetcodeFoundation/Math.h>

namespace Netcode::Graphics {

	struct Config {
		uint32_t width;
		uint32_t height;
		uint8_t graphicsAdapterIndex;
		uint8_t displayIndex;
		uint8_t backbufferDepth;
		bool fullscreen;

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			parentObject.AddMember("width", width, doc.GetAllocator());
			parentObject.AddMember("height", height, doc.GetAllocator());
			parentObject.AddMember("graphicsAdapterIndex", graphicsAdapterIndex, doc.GetAllocator());
			parentObject.AddMember("displayIndex", displayIndex, doc.GetAllocator());
			parentObject.AddMember("backbufferDepth", backbufferDepth, doc.GetAllocator());
			parentObject.AddMember("fullscreen", fullscreen, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			width = object["width"].GetUint();
			height = object["height"].GetUint();
			graphicsAdapterIndex = static_cast<uint8_t>(object["graphicsAdapterIndex"].GetUint());
			displayIndex = static_cast<uint8_t>(object["displayIndex"].GetUint());
			backbufferDepth = static_cast<uint8_t>(object["backbufferDepth"].GetUint());
			fullscreen = object["fullscreen"].GetBool();
		}

	};

}
