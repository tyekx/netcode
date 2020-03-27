#include "Input.h"

#if defined(EGG_OS_WINDOWS)
#include <Windows.h>
#endif

namespace Netcode {

	BYTE Input::inputBuffer[2048] = { 0 };

	bool Input::keysHeld[256] = { false };

	bool Input::isFocused{ false };
	DirectX::XMINT2 Input::mousePos{};
	DirectX::XMINT2 Input::mouseDelta{};
	std::map<std::string, Input::Axis> Input::axisMap{ };

	Event<uint32_t, uint32_t> Input::OnKeyPressed{};
	uint32_t Input::activeModifiers{ 0 };

	Input::Axis::Axis() : PositiveKey{ 0 }, NegativeKey{ 0 }, CurrentValue{ 0.0f } { }

	Input::Axis::Axis(uint32_t posK, uint32_t negK) : PositiveKey{ posK }, NegativeKey{ negK }, CurrentValue{ 0.0f } { }


	void Input::ReadRawMouse(unsigned long long wParam, unsigned long long lParam) {
		UINT dwSize = 0;

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

		ASSERT(dwSize < sizeof(inputBuffer), "Raw input must fit into the input buffer");

		if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, inputBuffer, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
			OutputDebugString("GetRawInputData does not return correct size !\n");
		}

		RAWINPUT * raw = (RAWINPUT *)inputBuffer;

		if(raw->header.dwType == RIM_TYPEMOUSE) {
			Input::MouseMove(DirectX::XMINT2{ raw->data.mouse.lLastX, raw->data.mouse.lLastY });
			if(raw->data.mouse.ulButtons != 0) {
				Netcode::Utility::Debugf("%d\r\n", raw->data.mouse.ulButtons);

				switch(raw->data.mouse.ulButtons) {
				case 1:
					Input::KeyPressed(VK_LBUTTON);
					break;
				case 2:
					Input::KeyReleased(VK_LBUTTON);
					break;
				}
			}
			auto v = DefRawInputProc(&raw, 1, sizeof(RAWINPUTHEADER));
			if(v != S_OK) {
				Netcode::Utility::Debugf("Input Error\r\n");
			}
		}
	}

	void Input::SetMousePos(const DirectX::XMINT2 & pos)
	{
		mousePos = pos;
	}

	void Input::CreateResources() {
		/*
		UINT numDevices = 0;
		GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));

		std::unique_ptr<RAWINPUTDEVICELIST[]> deviceList = std::make_unique<RAWINPUTDEVICELIST[]>(numDevices);

		UINT rv = GetRawInputDeviceList(deviceList.get(), &numDevices, sizeof(RAWINPUTDEVICELIST));

		

		for(UINT i = 0; i < numDevices; ++i) {
			RID_DEVICE_INFO devInfo;
			ZeroMemory(&devInfo, sizeof(RID_DEVICE_INFO));
			UINT bSize = sizeof(devInfo);
			HANDLE device = deviceList[i].hDevice;
			std::unique_ptr<char[]> strBuffer = std::make_unique<char[]>(256);
			ZeroMemory(strBuffer.get(), 256);
			UINT copiedData = GetRawInputDeviceInfo(device, RIDI_DEVICEINFO, &devInfo, &bSize);
			bSize = 256;
			copiedData = GetRawInputDeviceInfo(device, RIDI_DEVICENAME, strBuffer.get(), &bSize);
			Egg::Utility::Debugf("Device: %s: %d %d\r\n", strBuffer.get(), devInfo.dwType, copiedData);

			if(devInfo.dwType == RIM_TYPEMOUSE) {
				Egg::Utility::Debugf("\tId: %d | NumBtns: %d | Rate: %d Hz | \r\n", devInfo.mouse.dwId, devInfo.mouse.dwNumberOfButtons, devInfo.mouse.dwSampleRate);
			}
		}
		*/


		RAWINPUTDEVICE rawMouseDevice;
		rawMouseDevice.usUsagePage = 0x1;
		rawMouseDevice.usUsage = 0x2;
		rawMouseDevice.dwFlags = 0;
		rawMouseDevice.hwndTarget = 0;

		if(RegisterRawInputDevices(&rawMouseDevice, 1, sizeof(RAWINPUTDEVICE)) == FALSE) {
			OutputDebugString("Failed to connect raw input device\r\n");
			wchar_t buf[256];
			FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						   NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						   buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
			OutputDebugStringW(buf);
			__debugbreak();
		}

	}

	void Input::KeyPressed(uint32_t keyCode) {
		if(!isFocused) {
			return;
		}

		switch(keyCode) {
			case VK_SHIFT:
				activeModifiers |= static_cast<uint32_t>(KeyModifiers::SHIFT);
				break;
			case VK_CONTROL:
				activeModifiers |= static_cast<uint32_t>(KeyModifiers::CTRL);
				break;
			case VK_CAPITAL:
				activeModifiers |= static_cast<uint32_t>(KeyModifiers::CAPS_LOCK);
				break;
			case VK_MENU:
				activeModifiers |= static_cast<uint32_t>(KeyModifiers::ALT);
				break;
		}

		OnKeyPressed.Invoke(keyCode, activeModifiers);
		
		for(auto & i : axisMap) {
			if(i.second.PositiveKey == keyCode) {
				i.second.CurrentValue = 1.0f;
			} else if(i.second.NegativeKey == keyCode) {
				i.second.CurrentValue = -1.0f;
			}
		}
	}

	void Input::KeyReleased(uint32_t keyCode) {
		if(!isFocused) {
			return;
		}

		switch(keyCode) {
			case VK_SHIFT:
				activeModifiers &= (~static_cast<uint32_t>(KeyModifiers::SHIFT));
				break;
			case VK_CONTROL:
				activeModifiers &= (~static_cast<uint32_t>(KeyModifiers::CTRL));
				break;
			case VK_CAPITAL:
				activeModifiers &= (~static_cast<uint32_t>(KeyModifiers::CAPS_LOCK));
				break;
			case VK_MENU:
				activeModifiers &= (~static_cast<uint32_t>(KeyModifiers::ALT));
				break;
		}

		for(auto & i : axisMap) {
			if(i.second.PositiveKey == keyCode && i.second.CurrentValue > 0.0f) {
				i.second.CurrentValue = 0.0f;
			} else if(i.second.NegativeKey == keyCode && i.second.CurrentValue < 0.0f) {
				i.second.CurrentValue = 0.0f;
			}
		}
	}

	float Input::GetAxis(const std::string & axis) {
		decltype(axisMap)::const_iterator ci = axisMap.find(axis);

		if(ci != axisMap.end()) {
			return ci->second.CurrentValue;
		}

		Netcode::Utility::Debugf("Error: Axis '%s' was not found. Use SetAxis first", axis.c_str());

		return 0.0f;
	}

	void Input::SetAxis(const std::string & name, uint32_t posKey, uint32_t negKey) {
		decltype(axisMap)::const_iterator ci = axisMap.find(name);

		if(ci != axisMap.end()) {
			Netcode::Utility::Debugf("Notice: Axis '%s' was already set. Overwriting it");
		}

		axisMap[name] = Axis{ posKey, negKey };
	}

	void Input::MouseMove(const DirectX::XMINT2 & xy) {
		mouseDelta = DirectX::XMINT2{ mouseDelta.x + xy.x, mouseDelta.y + xy.y };
	}

	DirectX::XMINT2 Input::GetMouseDelta() {
		return mouseDelta;
	}

	DirectX::XMINT2 Input::GetMousePos()
	{
		return mousePos;
	}

	void Input::Blur() {
		isFocused = false;
		activeModifiers = 0;
		//SetCursor(LoadCursor(NULL, IDC_ARROW));
	}

	void Input::Focused() {
		mouseDelta = { 0, 0 };
		isFocused = true;
		activeModifiers = 0;
		//SetCursor(NULL);
	}

	void Input::Reset() {
		//if(IsFocused) {
			//SetCursorPos(960, 540);
		//}
		mouseDelta = DirectX::XMINT2{ 0,0 };
	}

}
