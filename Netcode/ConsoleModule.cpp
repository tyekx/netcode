#include "ConsoleModule.h"
#include <iostream>

namespace Netcode::Module {
	void ConsoleModule::Start(AApp * app) {
		keepRunning = true;
		thread = std::make_unique<std::thread>([this]()-> void {
			std::string line;
			while(keepRunning.load() && std::getline(std::cin, line)) {
				mutex.lock();
				commands.push_back(line);
				mutex.unlock();
			}
		});
	}

	void ConsoleModule::Shutdown() {
		if(!keepRunning)
		{
			return;
		}

		keepRunning = false;

		DWORD dwTmp;
		INPUT_RECORD ir[2];
		ir[0].EventType = KEY_EVENT;
		ir[0].Event.KeyEvent.bKeyDown = TRUE;
		ir[0].Event.KeyEvent.dwControlKeyState = 0;
		ir[0].Event.KeyEvent.uChar.UnicodeChar = VK_RETURN;
		ir[0].Event.KeyEvent.wRepeatCount = 1;
		ir[0].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
		ir[0].Event.KeyEvent.wVirtualScanCode = MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC);
		ir[1] = ir[0];
		ir[1].Event.KeyEvent.bKeyDown = FALSE;
		WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), ir, 2, &dwTmp);

		thread->join();
	}

	void * ConsoleModule::GetUnderlyingPointer() {
		return nullptr;
	}

	void ConsoleModule::ProcessMessages() {

		for(const auto & i : commands) {
			if(i == "exit") {
				Shutdown();
				break;
			}
		}

		mutex.lock();
		commands.clear();
		mutex.unlock();
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
