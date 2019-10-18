#include "Input.h"

namespace Egg {

	BYTE Input::InputBuffer[2048] = { 0 };

	bool Input::IsFocused{ false };
	DirectX::XMINT2 Input::LastMousePos{ -1, -1 };
	DirectX::XMINT2 Input::MouseDelta{};
	std::map<std::string, Input::Axis> Input::AxisMap{ };

	Input::Axis::Axis() : PositiveKey{ 0 }, NegativeKey{ 0 }, CurrentValue{ 0.0f } { }

	Input::Axis::Axis(uint32_t posK, uint32_t negK) : PositiveKey{ posK }, NegativeKey{ negK }, CurrentValue{ 0.0f } { }


	void Input::ReadRawMouse(WPARAM wParam, LPARAM lParam) {
		UINT dwSize = 0;

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

		ASSERT(dwSize < sizeof(InputBuffer), "Raw input must fit into the input buffer");

		if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, InputBuffer, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
			OutputDebugString("GetRawInputData does not return correct size !\n");
		}

		RAWINPUT * raw = (RAWINPUT *)InputBuffer;

		if(raw->header.dwType == RIM_TYPEMOUSE) {
			Input::MouseMove(DirectX::XMINT2{ raw->data.mouse.lLastX, raw->data.mouse.lLastY });
			if(raw->data.mouse.ulButtons != 0) {
				Egg::Utility::Debugf("%d\r\n", raw->data.mouse.ulButtons);

				switch(raw->data.mouse.ulButtons) {
				case 1:
					Input::KeyPressed(VK_LBUTTON);
					break;
				case 2:
					Input::KeyReleased(VK_LBUTTON);
					break;

				}
			}
		}
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
		}*/



		RAWINPUTDEVICE rawMouseDevice;
		rawMouseDevice.usUsagePage = 0x1;
		rawMouseDevice.usUsage = 0x2;
		rawMouseDevice.dwFlags = RIDEV_NOLEGACY;
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
		if(!IsFocused) {
			return;
		}
		for(auto & i : AxisMap) {
			if(i.second.PositiveKey == keyCode) {
				i.second.CurrentValue = 1.0f;
			} else if(i.second.NegativeKey == keyCode) {
				i.second.CurrentValue = -1.0f;
			}
		}
	}

	void Input::KeyReleased(uint32_t keyCode) {
		if(!IsFocused) {
			return;
		}
		for(auto & i : AxisMap) {
			if(i.second.PositiveKey == keyCode && i.second.CurrentValue > 0.0f) {
				i.second.CurrentValue = 0.0f;
			} else if(i.second.NegativeKey == keyCode && i.second.CurrentValue < 0.0f) {
				i.second.CurrentValue = 0.0f;
			}
		}
	}

	float Input::GetAxis(const std::string & axis) {
		decltype(AxisMap)::const_iterator ci = AxisMap.find(axis);

		if(ci != AxisMap.end()) {
			return ci->second.CurrentValue;
		}

		Egg::Utility::Debugf("Error: Axis '%s' was not found. Use SetAxis first", axis.c_str());

		return 0.0f;
	}

	void Input::SetAxis(const std::string & name, uint32_t posKey, uint32_t negKey) {
		decltype(AxisMap)::const_iterator ci = AxisMap.find(name);

		if(ci != AxisMap.end()) {
			Egg::Utility::Debugf("Notice: Axis '%s' was already set. Overwriting it");
		}

		AxisMap[name] = Axis{ posKey, negKey };
	}

	void Input::MouseMove(const DirectX::XMINT2 & xy) {
		MouseDelta = DirectX::XMINT2{ MouseDelta.x + xy.x, MouseDelta.y + xy.y };
	}

	DirectX::XMINT2 Input::GetMouseDelta() {
		return MouseDelta;
	}

	void Input::Blur() {
		LastMousePos = DirectX::XMINT2{ -1, -1 };
		IsFocused = false;
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}

	void Input::Focused() {
		IsFocused = true;
		SetCursor(NULL);
	}

	void Input::Reset() {
		if(IsFocused) {
			SetCursorPos(960, 540);
		}
		MouseDelta = DirectX::XMINT2{ 0,0 };
	}

}
