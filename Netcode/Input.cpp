#include "Input.h"

#if defined(NETCODE_OS_WINDOWS)
#include <Windows.h>
#include <hidusage.h>
#endif

namespace Netcode {

	inline bool operator>=(uint32_t lhs, KeyCode rhs) {
		return lhs >= static_cast<uint32_t>(rhs);
	}

	inline bool operator<=(uint32_t lhs, KeyCode rhs) {
		return lhs <= static_cast<uint32_t>(rhs);
	}

	inline bool operator<(uint32_t lhs, KeyCode rhs) {
		return lhs < static_cast<uint32_t>(rhs);
	}

	inline bool operator>(uint32_t lhs, KeyCode rhs) {
		return lhs > static_cast<uint32_t>(rhs);
	}

	inline bool operator==(uint32_t lhs, KeyCode rhs) {
		return lhs == static_cast<uint32_t>(rhs);
	}

	inline bool operator!=(uint32_t lhs, KeyCode rhs) {
		return lhs != static_cast<uint32_t>(rhs);
	}

	static KeyCode MapKeyCode(uint32_t keyValue) {
		if(keyValue > KeyCode::SCROLL_LOCK) {
			return KeyCode::UNDEFINED;
		}

		return static_cast<KeyCode>(keyValue);
	}

	struct Input::detail {
	private:
		Alloc allocator;

		AxisMapRef axisMap;

	public:
		EventType<Key, KeyModifier> OnInput;
		EventType<Key, KeyModifier> OnMouseInput;
		EventType<Key, KeyModifier> OnMouseKeyPressed;
		EventType<Key, KeyModifier> OnMouseKeyReleased;
		EventType<Key, KeyModifier> OnKeyInput;
		EventType<Key, KeyModifier> OnKeyPressed;
		EventType<Key, KeyModifier> OnKeyReleased;

		EventType<int, KeyModifier> OnScroll;
		EventType<Int2, KeyModifier> OnMouseMove;

	private:
		Int2 mouseDelta;
		Int2 mouseWindowPosition;

		Key keys[256];

		uint8_t inputBuffer[2048];
		
		void InputEvent(Key keyEvt) {
			OnInput.Invoke(keyEvt, KeyModifier::NONE);

			if(keyEvt.IsKeyboard()) {
				OnKeyInput.Invoke(keyEvt, KeyModifier::NONE);

				if(keyEvt.IsRising() || keyEvt.GetState() == KeyState::PULSE) {
					OnKeyPressed.Invoke(keyEvt, KeyModifier::NONE);
				}

				if(keyEvt.IsFalling()) {
					OnKeyReleased.Invoke(keyEvt, KeyModifier::NONE);
				}
			}

			if(keyEvt.IsMouse()) {
				OnMouseInput.Invoke(keyEvt, KeyModifier::NONE);

				if(keyEvt.IsFalling()) {
					OnMouseKeyReleased.Invoke(keyEvt, KeyModifier::NONE);
				}

				if(keyEvt.IsRising()) {
					OnMouseKeyPressed.Invoke(keyEvt, KeyModifier::NONE);
				}
			}
		}

	public:
		detail() : allocator{  },
			OnInput{ StdAlloc{ allocator } },
			OnMouseInput{ StdAlloc{ allocator } },
			OnMouseKeyPressed{ StdAlloc{ allocator } },
			OnMouseKeyReleased{ StdAlloc{ allocator } },
			OnKeyInput{ StdAlloc{ allocator } },
			OnKeyPressed{ StdAlloc{ allocator } },
			OnKeyReleased{ StdAlloc{ allocator } },
			OnScroll{ StdAlloc{ allocator } },
			OnMouseMove{ StdAlloc{ allocator } },
			mouseDelta{ Int2::Zero },
			mouseWindowPosition{ Int2::Zero },
			keys{ },
			inputBuffer{} {

			for(uint32_t i = 0; i < 256; ++i) {
				keys[i] = Key(MapKeyCode(i), KeyState::INACTIVE);
			}

		}

		void ProcessEvent(Key keyEvent) {
			uint32_t idx = static_cast<uint32_t>(keyEvent.GetCode());
			Key currentState = keys[idx];

			if(currentState.IsReleased() && keyEvent.GetState() == KeyState::ACTIVE) {
				keyEvent.SetState(KeyState::RISING_EDGE);
				keys[idx].SetState(KeyState::RISING_EDGE);
			}

			if(currentState.IsPressed() && keyEvent.GetState() == KeyState::INACTIVE) {
				keyEvent.SetState(KeyState::FALLING_EDGE);
				keys[idx].SetState(KeyState::FALLING_EDGE);
			}

			if(keyEvent.GetState() == KeyState::ACTIVE && currentState.GetState() == KeyState::ACTIVE) {
				keyEvent.SetState(KeyState::PULSE);
			}

			InputEvent(keyEvent);
		}

		void ProcessMouseScrollEvent(int delta) {
			OnScroll.Invoke(delta, KeyModifier::NONE);
		}


		void ProcessMouseMoveEvent(const Int2 & mouseWindowPosition)
		{
			this->mouseWindowPosition = mouseWindowPosition;

			OnMouseMove.Invoke(mouseWindowPosition, KeyModifier::NONE);
		}

		void CompleteFrame() {
			for(uint32_t i = 0; i < 256; ++i) {
				Key * key = keys + i;

				if(key->IsFalling()) {
					key->SetState(KeyState::INACTIVE);
				}

				if(key->IsRising()) {
					key->SetState(KeyState::ACTIVE);
				}
			}

			mouseDelta = Int2::Zero;
		}

		void Initialize() {
			RAWINPUTDEVICE rawMouseDevice;
			rawMouseDevice.usUsagePage = HID_USAGE_PAGE_GENERIC;
			rawMouseDevice.usUsage = HID_USAGE_GENERIC_MOUSE;
			rawMouseDevice.dwFlags = 0;
			rawMouseDevice.hwndTarget = NULL;

			BOOL success = RegisterRawInputDevices(&rawMouseDevice, 1, sizeof(RAWINPUTDEVICE));

			UndefinedBehaviourAssertion(success == TRUE);
		}

		void UpdateAxisMap(float dt) {
			UNREFERENCED_PARAMETER(dt);

			if(axisMap != nullptr) {
				axisMap->Update(ArrayView<Key>(keys, 256));
			}
		}

		float GetAxis(uint32_t axis) {
			if(axisMap != nullptr) {
				return axisMap->GetAxis(axis);
			} 
			return 0.0f;
		}

		Int2 GetMouseDelta()
		{
			return mouseDelta;
		}

		Int2 GetMousePosition()
		{
			return mouseWindowPosition;
		}

		void SetAxisMap(AxisMapRef axMap) {
			axisMap = std::move(axMap);
		}

		void ReadWinApiMouse(uintptr_t wParam, intptr_t lParam) {
			static_assert(std::is_same_v<uintptr_t, WPARAM> && std::is_same_v<intptr_t, LPARAM>, "Mismatching API types");

			UINT dwSize = 0;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

			UndefinedBehaviourAssertion(dwSize < sizeof(inputBuffer));

			const UINT writtenSize = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, inputBuffer, &dwSize, sizeof(RAWINPUTHEADER));

			UndefinedBehaviourAssertion(writtenSize == dwSize);

			RAWINPUT * rawInput = (RAWINPUT *)inputBuffer;

			if(rawInput->header.dwType == RIM_TYPEMOUSE) {

				RAWMOUSE * rawMouse = &rawInput->data.mouse;
				
				UndefinedBehaviourAssertion((rawMouse->usFlags & MOUSE_MOVE_ABSOLUTE) != MOUSE_MOVE_ABSOLUTE);

				mouseDelta = Int2{ mouseDelta.x + rawMouse->lLastX, mouseDelta.y + rawMouse->lLastY };

				if(rawMouse->ulButtons != 0) {
#define IF_PRESENT(numValue) if((rawMouse->ulButtons & numValue) == numValue)
					IF_PRESENT(RI_MOUSE_LEFT_BUTTON_DOWN) ProcessEvent(Key{ KeyCode::MOUSE_LEFT, KeyState::ACTIVE });
					IF_PRESENT(RI_MOUSE_LEFT_BUTTON_UP) ProcessEvent(Key{ KeyCode::MOUSE_LEFT, KeyState::INACTIVE });

					IF_PRESENT(RI_MOUSE_RIGHT_BUTTON_DOWN) ProcessEvent(Key{ KeyCode::MOUSE_RIGHT, KeyState::ACTIVE });
					IF_PRESENT(RI_MOUSE_RIGHT_BUTTON_UP) ProcessEvent(Key{ KeyCode::MOUSE_RIGHT, KeyState::INACTIVE });

					IF_PRESENT(RI_MOUSE_MIDDLE_BUTTON_DOWN) ProcessEvent(Key{ KeyCode::MOUSE_MIDDLE, KeyState::ACTIVE });
					IF_PRESENT(RI_MOUSE_MIDDLE_BUTTON_UP) ProcessEvent(Key{ KeyCode::MOUSE_MIDDLE, KeyState::INACTIVE });

					IF_PRESENT(RI_MOUSE_BUTTON_4_DOWN) ProcessEvent(Key{ KeyCode::MOUSE4, KeyState::ACTIVE });
					IF_PRESENT(RI_MOUSE_BUTTON_4_UP) ProcessEvent(Key{ KeyCode::MOUSE4, KeyState::INACTIVE });

					IF_PRESENT(RI_MOUSE_BUTTON_5_DOWN) ProcessEvent(Key{ KeyCode::MOUSE5, KeyState::ACTIVE });
					IF_PRESENT(RI_MOUSE_BUTTON_5_UP) ProcessEvent(Key{ KeyCode::MOUSE5, KeyState::INACTIVE });

					IF_PRESENT(RI_MOUSE_WHEEL) ProcessMouseScrollEvent(static_cast<int16_t>(rawMouse->usButtonData));
#undef IF_PRESENT
				}

				LRESULT lResult = DefRawInputProc(&rawInput, 1, sizeof(RAWINPUTHEADER));

				UndefinedBehaviourAssertion(SUCCEEDED(lResult));
			}

		}

	};


	std::unique_ptr<Input::detail> Input::instance{ nullptr };

	Input::EventType<Key, KeyModifier> * Input::OnInput{ nullptr };
	Input::EventType<Key, KeyModifier> * Input::OnMouseInput{ nullptr };
	Input::EventType<Key, KeyModifier> * Input::OnMouseKeyPressed{ nullptr };
	Input::EventType<Key, KeyModifier> * Input::OnMouseKeyReleased{ nullptr };
	Input::EventType<Key, KeyModifier> * Input::OnKeyInput{ nullptr };
	Input::EventType<Key, KeyModifier> * Input::OnKeyPressed{ nullptr };
	Input::EventType<Key, KeyModifier> * Input::OnKeyReleased{ nullptr };
	Input::EventType<int, KeyModifier> * Input::OnScroll{ nullptr };
	Input::EventType<Int2, KeyModifier> * Input::OnMouseMove{ nullptr };

	void Input::ProcessEvent(Key keyEvent) {
		instance->ProcessEvent(keyEvent);
	}

	void Input::ProcessMouseMoveEvent(const Int2 & mouseWindowPosition)
	{
		instance->ProcessMouseMoveEvent(mouseWindowPosition);
	}

	void Input::ProcessPlatformEvent(uintptr_t wParam, intptr_t lParam)
	{
		instance->ReadWinApiMouse(wParam, lParam);
	}

	void Input::Initialize() {
		if(instance == nullptr) {
			instance = std::make_unique<Input::detail>();

			OnInput = &instance->OnInput;
			OnMouseInput = &instance->OnMouseInput;
			OnMouseKeyPressed = &instance->OnMouseKeyPressed;
			OnMouseKeyReleased = &instance->OnMouseKeyReleased;
			OnKeyInput = &instance->OnKeyInput;
			OnKeyPressed = &instance->OnKeyPressed;
			OnKeyReleased = &instance->OnKeyReleased;

			OnScroll = &instance->OnScroll;
			OnMouseMove = &instance->OnMouseMove;

			instance->Initialize();
		}
	}

	void Input::UpdateAxisMap(float dt) {
		instance->UpdateAxisMap(dt);
	}

	float Input::GetAxis(uint32_t axis) {
		return instance->GetAxis(axis);
	}

	void Input::SetAxisMap(AxisMapRef axisMap) {
		instance->SetAxisMap(std::move(axisMap));
	}

	void Input::CompleteFrame()
	{
		instance->CompleteFrame();
	}

	Int2 Input::GetMouseDelta()
	{
		return instance->GetMouseDelta();
	}

	Int2 Input::GetMousePosition()
	{
		return instance->GetMousePosition();
	}

}
