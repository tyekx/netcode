#include "FrameGraph.h"

namespace Egg {

	RenderPass::RenderPass(const std::string & name, SetupCallback setupFunc, RenderCallback renderFunc) : name{ name }, Setup{ setupFunc }, Render{ renderFunc } {

	}
}
