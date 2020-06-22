#pragma once


#include <NetcodeFoundation/Formats.h>
#include <NetcodeFoundation/Math.h>

#include <rapidjson/document.h>

#include "../ConfigBase.h"

namespace Netcode::Graphics {

	struct Config : public ConfigBase {
		DXGI_FORMAT backbufferFormat;
		DXGI_FORMAT depthStencilFormat;
		uint8_t displayIndex;
		uint8_t adapterIndex;
		float targetFps;

		Config(Ptree & parentNode) : ConfigBase{ parentNode, "graphics" } { }

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			parentObject.AddMember("backbufferFormat", static_cast<uint32_t>(backbufferFormat), doc.GetAllocator());
			parentObject.AddMember("depthStencilFormat", static_cast<uint32_t>(depthStencilFormat), doc.GetAllocator());
			parentObject.AddMember("displayIndex", displayIndex, doc.GetAllocator());
			parentObject.AddMember("adapterIndex", adapterIndex, doc.GetAllocator());
			parentObject.AddMember("targetFps", targetFps, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			backbufferFormat = static_cast<DXGI_FORMAT>(object["backbufferFormat"].GetUint());
			depthStencilFormat = static_cast<DXGI_FORMAT>(object["depthStencilFormat"].GetUint());
			adapterIndex = static_cast<uint8_t>(object["adapterIndex"].GetUint());
			displayIndex = static_cast<uint8_t>(object["displayIndex"].GetUint());
			targetFps = object["targetFps"].GetFloat();

			node->put("backbufferFormat", Property{ backbufferFormat });
			node->put("depthStencilFormat", Property{ depthStencilFormat });
			node->put("adapterIndex", Property{ adapterIndex });
			node->put("displayIndex", Property{ displayIndex });
			node->put("targetFps", Property{ targetFps });
		}

	};

}
