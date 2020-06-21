#pragma once

#include <NetcodeFoundation/Formats.h>
#include <NetcodeFoundation/Math.h>

#include <rapidjson/document.h>

namespace Netcode::Graphics {

	struct Config {
		DXGI_FORMAT backbufferFormat;
		DXGI_FORMAT depthStencilFormat;
		uint8_t displayIndex;
		uint8_t adapterIndex;
		float targetFps;

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			parentObject.AddMember("backbufferFormat", static_cast<uint32_t>(backbufferFormat), doc.GetAllocator());
			parentObject.AddMember("depthStencilFormat", static_cast<uint32_t>(depthStencilFormat), doc.GetAllocator());
			parentObject.AddMember("adapterIndex", adapterIndex, doc.GetAllocator());
			parentObject.AddMember("targetFps", targetFps, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			backbufferFormat = static_cast<DXGI_FORMAT>(object["backbufferFormat"].GetUint());
			depthStencilFormat = static_cast<DXGI_FORMAT>(object["depthStencilFormat"].GetUint());
			adapterIndex = static_cast<uint8_t>(object["graphicsAdapterIndex"].GetUint());
			displayIndex = static_cast<uint8_t>(object["displayIndex"].GetUint());
			targetFps = object["targetFps"].GetFloat();
		}

	};

}
