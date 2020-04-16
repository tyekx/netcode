#include "ConsoleModule.h"
#include <iostream>

namespace Netcode::Module {
	void ConsoleModule::Start(AApp * app)
	{
		keepRunning = true;
	}

	void ConsoleModule::Shutdown()
	{
		keepRunning = false;
	}
	void * ConsoleModule::GetUnderlyingPointer() {
		return nullptr;
	}

	void ConsoleModule::ProcessMessages() {
		if(std::cin.peek() == EOF) {
			keepRunning = false;
		}
	}

	void ConsoleModule::CompleteFrame() {

	}

	void ConsoleModule::ShowWindow() {

	}

	bool ConsoleModule::KeepRunning() {
		return keepRunning;
	}

	void ConsoleModule::ShowDebugWindow() {

	}

}
