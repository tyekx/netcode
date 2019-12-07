#include "Modules.h"

namespace Egg::Module {
	void AApp::StartModule(IModule * m) {
		if(m) {
			m->Start(this);
		}
	}
	void AApp::ShutdownModule(IModule * m) {
		if(m) {
			m->Shutdown();
		}
	}
}

