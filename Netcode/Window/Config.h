#pragma once

#include <rapidjson/document.h>

#include <NetcodeFoundation/Math.h>

namespace Netcode::Window {

	struct Config {
		Int2 defaultSize;

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {

		}

		void Load(const rapidjson::Value & object) {

		}
	};

}
