#pragma once

#include <rapidjson/document.h>

#include <NetcodeFoundation/Math.h>

#include "../ConfigBase.h"
#include "../MathExt.h"

namespace Netcode::Window {

	struct Config : public ConfigBase {
		Int2 size;

		Config(Ptree &parentNode) : ConfigBase { parentNode, "window" } { }

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			
		}

		void Load(const rapidjson::Value & object) {
			size = LoadInt2(object["size"]);

			node->put("size", Property{ size });
		}
	};

}
