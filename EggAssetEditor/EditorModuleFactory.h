#pragma once

#include <Egg/DefaultModuleFactory.h>

namespace Egg::Module {

	class EditorModuleFactory : public DefaultModuleFactory {
	public:
		virtual std::unique_ptr<IGraphicsModule> CreateGraphicsModule(AApp * app, int graphicsType) override;
	};

}
