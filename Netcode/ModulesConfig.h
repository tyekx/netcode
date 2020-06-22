#pragma once

#include <rapidjson/document.h>

#include "Common.h"
#include "ConfigBase.h"
#include "Graphics/Config.h"
#include "Network/Config.h"
#include "Window/Config.h"

/*
with build time definitions you can control the compiled modules. By default every buildable module is included in the build.

Define
NETCODE_BUILD_<name>_MODULE: to explicitly build a module, if its not possible, the build will not succeed
NETCODE_EXCLUDE_<name>_MODULE: to explicitly exclude a module from the build

Not having any built module for a specific type of modules is not considered to be an error, as every interface has an empty "Null" implementation to fall back to.
*/

#if !defined(NETCODE_EXCLUDE_DX12_MODULE)
#define DETAIL_BUILD_DX12_MODULE
#endif

#if defined(DETAIL_BUILD_DX12_MODULE)

	#if defined(NETCODE_OS_WINDOWS) && !defined(NETCODE_EXCLUDE_DX12_MODULE)

		#define NETCODE_BUILD_DX12_MODULE

	#else

		#pragma warning "The platform is not windows, DX12 module will be ommitted"

	#endif

#endif

namespace Netcode {

	struct Config {
		Ptree root;

		Network::Config network;
		Graphics::Config graphics;
		Window::Config window;

		Config() :root{}, network { root }, graphics{ root }, window{ root } { }

		Config(const Config &) = delete;
		Config(Config && rhs) noexcept = default;

		Config & operator=(Config cfg) noexcept {
			std::swap(root, cfg.root);
			std::swap(network, cfg.network);
			std::swap(graphics, cfg.graphics);
			std::swap(window, cfg.window);
			return *this;
		}

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			rapidjson::Value networkObj{ rapidjson::kObjectType };
			rapidjson::Value graphicsObj{ rapidjson::kObjectType };
			rapidjson::Value windowObj{ rapidjson::kObjectType };

			network.Store(networkObj, doc);
			graphics.Store(graphicsObj, doc);
			window.Store(windowObj, doc);

			parentObject.AddMember("network", networkObj, doc.GetAllocator());
			parentObject.AddMember("graphics", graphicsObj, doc.GetAllocator());
			parentObject.AddMember("window", windowObj, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			network.Load(object["network"]);
			graphics.Load(object["graphics"]);
			window.Load(object["window"]);
		}

	};

}
