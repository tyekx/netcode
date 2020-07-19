#include "Input.h"

#if defined(NETCODE_OS_WINDOWS)
#include <Windows.h>
#include <hidusage.h>
#endif

namespace Netcode {

	static KeyCode iterableKeyCodes[] = { KeyCode::MOUSE_LEFT,
		KeyCode::MOUSE_RIGHT,
		KeyCode::MOUSE_MIDDLE,
		KeyCode::MOUSE4,
		KeyCode::MOUSE5,
		KeyCode::BACKSPACE,
		KeyCode::TAB,
		KeyCode::CLEAR,
		KeyCode::RETURN,
		KeyCode::SHIFT,
		KeyCode::CONTROL,
		KeyCode::ALT,
		KeyCode::PAUSE,
		KeyCode::CAPS_LOCK,
		KeyCode::ESCAPE,
		KeyCode::SPACE,
		KeyCode::PAGE_UP,
		KeyCode::PAGE_DOWN,
		KeyCode::END,
		KeyCode::HOME,
		KeyCode::LEFT,
		KeyCode::UP,
		KeyCode::RIGHT,
		KeyCode::DOWN,
		KeyCode::PRINT_SCREEN,
		KeyCode::INSERT,
		KeyCode::DEL,
		KeyCode::_0,
		KeyCode::_1,
		KeyCode::_2,
		KeyCode::_3,
		KeyCode::_4,
		KeyCode::_5,
		KeyCode::_6,
		KeyCode::_7,
		KeyCode::_8,
		KeyCode::_9,
		KeyCode::A,
		KeyCode::B,
		KeyCode::C,
		KeyCode::D,
		KeyCode::E,
		KeyCode::F,
		KeyCode::G,
		KeyCode::H,
		KeyCode::I,
		KeyCode::J,
		KeyCode::K,
		KeyCode::L,
		KeyCode::M,
		KeyCode::N,
		KeyCode::O,
		KeyCode::P,
		KeyCode::Q,
		KeyCode::R,
		KeyCode::S,
		KeyCode::T,
		KeyCode::U,
		KeyCode::V,
		KeyCode::W,
		KeyCode::X,
		KeyCode::Y,
		KeyCode::Z,
		KeyCode::NUM_0,
		KeyCode::NUM_1,
		KeyCode::NUM_2,
		KeyCode::NUM_3,
		KeyCode::NUM_4,
		KeyCode::NUM_5,
		KeyCode::NUM_6,
		KeyCode::NUM_7,
		KeyCode::NUM_8,
		KeyCode::NUM_9,
		KeyCode::MULTIPLY,
		KeyCode::ADD,
		KeyCode::SEPARATOR,
		KeyCode::SUBTRACT,
		KeyCode::DECIMAL,
		KeyCode::DIVIDE,
		KeyCode::F1,
		KeyCode::F2,
		KeyCode::F3,
		KeyCode::F4,
		KeyCode::F5,
		KeyCode::F6,
		KeyCode::F7,
		KeyCode::F8,
		KeyCode::F9,
		KeyCode::F10,
		KeyCode::F11,
		KeyCode::F12,
		KeyCode::NUM_LOCK,
		KeyCode::SCROLL_LOCK,
		KeyCode::SHIFT_LEFT,
		KeyCode::SHIFT_RIGHT,
		KeyCode::CONTROL_LEFT,
		KeyCode::CONTROL_RIGHT,
		KeyCode::ALT_LEFT,
		KeyCode::ALT_RIGHT 
	};

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
		if(keyValue > KeyCode::ALT_RIGHT) {
			return KeyCode::UNDEFINED;
		}

		return static_cast<KeyCode>(keyValue);
	}

	static KeyModifier MapKeyModifier(KeyCode code) {
		switch(code) {
			case KeyCode::ALT_LEFT: [[fallthrough]];
			case KeyCode::ALT_RIGHT: [[fallthrough]];
			case KeyCode::ALT: return KeyModifier::ALT;

			case KeyCode::SHIFT_LEFT: [[fallthrough]];
			case KeyCode::SHIFT_RIGHT: [[fallthrough]];
			case KeyCode::SHIFT: return KeyModifier::SHIFT;

			case KeyCode::CONTROL_LEFT: [[fallthrough]];
			case KeyCode::CONTROL_RIGHT: [[fallthrough]];
			case KeyCode::CONTROL: return KeyModifier::CTRL;

			default: return KeyModifier::NONE;
		}
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
		EventType<wchar_t> OnCharInput;

	private:

		class KeyStorage {
			Key keys[256];

		public:
			KeyStorage() : keys{} {
				for(uint32_t i = 0; i < 256; ++i) {
					keys[i] = Key(MapKeyCode(i), KeyState::RELEASED);
				}
			}

			const Key & operator[](KeyCode code) const {
				return keys[static_cast<uint32_t>(code)];
			}

			Key & operator[](KeyCode code) {
				return keys[static_cast<uint32_t>(code)];
			}

			ArrayView<Key> GetView() const {
				return ArrayView<Key>{keys, 256};
			}
		};

		Int2 mouseDelta;
		Int2 mouseWindowPosition;
		KeyModifier modifiers;
		KeyStorage keys;

		uint8_t inputBuffer[2048];
		
		void InputEvent(Key keyEvt) {
			OnInput.Invoke(keyEvt, modifiers);

			if(keyEvt.IsKeyboard()) {
				OnKeyInput.Invoke(keyEvt, modifiers);

				if(keyEvt.IsRising() || keyEvt.IsPulse()) {
					OnKeyPressed.Invoke(keyEvt, modifiers);
				}

				if(keyEvt.IsFalling()) {
					OnKeyReleased.Invoke(keyEvt, modifiers);
				}
			}

			if(keyEvt.IsMouse()) {
				OnMouseInput.Invoke(keyEvt, modifiers);

				if(keyEvt.IsFalling()) {
					OnMouseKeyReleased.Invoke(keyEvt, modifiers);
				}

				if(keyEvt.IsRising()) {
					OnMouseKeyPressed.Invoke(keyEvt, modifiers);
				}
			}
		}

		void ReflectKeyboardState() {
			static BYTE keyStates[256] = {};

			BOOL queryResult = GetKeyboardState(keyStates);

			if(queryResult) {
				for(uint32_t i = 0; i < ARRAYSIZE(iterableKeyCodes); ++i) {
					KeyCode keyCode = iterableKeyCodes[i];
					BYTE st = keyStates[static_cast<uint32_t>(keyCode)];

					bool isKeyDown = (st & 0x80) == 0x80;
					bool isKeyToggled = (st & 0x01) == 0x01;


					if(isKeyDown) {
						keys[keyCode].MergeStates(KeyState::PRESSED);
					} else {
						keys[keyCode].RemoveStates(KeyState::PRESSED);
					}

					if(IsModifier(keyCode)) {
						KeyModifier modifier = MapKeyModifier(keyCode);

						if(isKeyDown) {
							modifiers = modifiers | modifier;
						} else {
							modifiers = modifiers & static_cast<KeyModifier>(~static_cast<uint32_t>(modifier));
						}
					}

					if(IsToggledType(keyCode)) {
						if(isKeyToggled) {
							keys[keyCode].MergeStates(KeyState::TOGGLED);
						} else {
							keys[keyCode].RemoveStates(KeyState::TOGGLED);
						}
					}
				}
			}

			UpdateModifiers();
		}

		static KeyState ApplyModifierLogic(Key leftModifier, Key rightModifier) {
			KeyState leftState = leftModifier.GetState();
			KeyState rightState = rightModifier.GetState();

			UndefinedBehaviourAssertion(!KeyStateContains(leftState, KeyState::PULSE) &&
										!KeyStateContains(leftState, KeyState::TOGGLED) &&
										!KeyStateContains(rightState, KeyState::PULSE) &&
										!KeyStateContains(rightState, KeyState::TOGGLED));

			return leftState | rightState;
		}

		void UpdateModifiers() {
			Key leftAlt = keys[KeyCode::ALT_LEFT];
			Key rightAlt = keys[KeyCode::ALT_RIGHT];
			
			keys[KeyCode::ALT].SetState(ApplyModifierLogic(leftAlt, rightAlt));
			
			Key leftControl = keys[KeyCode::CONTROL_LEFT];
			Key rightControl = keys[KeyCode::CONTROL_RIGHT];

			keys[KeyCode::CONTROL].SetState(ApplyModifierLogic(leftControl, rightControl));

			Key leftShift = keys[KeyCode::SHIFT_LEFT];
			Key rightShift = keys[KeyCode::SHIFT_RIGHT];

			keys[KeyCode::SHIFT].SetState(ApplyModifierLogic(leftShift, rightShift));

			KeyModifier sum = KeyModifier::NONE;

			if(keys[KeyCode::ALT].IsPressed()) {
				sum = sum | KeyModifier::ALT;
			}

			if(keys[KeyCode::CONTROL].IsPressed()) {
				sum = sum | KeyModifier::CTRL;
			}

			if(keys[KeyCode::SHIFT].IsPressed()) {
				sum = sum | KeyModifier::SHIFT;
			}

			modifiers = sum;
		}

	public:
		detail() : allocator{  },
			axisMap{ nullptr },
			OnInput{ StdAlloc{ allocator } },
			OnMouseInput{ StdAlloc{ allocator } },
			OnMouseKeyPressed{ StdAlloc{ allocator } },
			OnMouseKeyReleased{ StdAlloc{ allocator } },
			OnKeyInput{ StdAlloc{ allocator } },
			OnKeyPressed{ StdAlloc{ allocator } },
			OnKeyReleased{ StdAlloc{ allocator } },
			OnScroll{ StdAlloc{ allocator } },
			OnMouseMove{ StdAlloc{ allocator } },
			OnCharInput{ StdAlloc{ allocator } },
			mouseDelta{ Int2::Zero },
			mouseWindowPosition{ Int2::Zero },
			modifiers{ KeyModifier::NONE },
			keys{ },
			inputBuffer{} {

			ReflectKeyboardState();
		}

		void ProcessBlurredEvent() {

		}

		void ProcessFocusedEvent() {
			ReflectKeyboardState();
		}

		void ProcessCharEvent(wchar_t value) {
			OnCharInput.Invoke(value);
		}

		void ProcessKeyEvent(Key keyEvent) {
			KeyCode keyCode = keyEvent.GetCode();
			Key propagatedKeyEvent{ keyEvent.GetCode(), KeyState::UNDEFINED };
			Key currentState = keys[keyCode];

			/**
			* 1.) Pulse state does not get saved, its only for propagation
			* 2.) Toggled bit is handled separately for ToggledTypes.
			*/

			if(currentState.IsReleased() && keyEvent.IsPressed()) {
				keys[keyCode].MergeStates(KeyState::RISING_EDGE);

				if(IsToggledType(keyEvent.GetCode())) {
					if(currentState.IsToggled()) {
						keys[keyCode].RemoveStates(KeyState::TOGGLED);
					} else {
						keys[keyCode].MergeStates(KeyState::TOGGLED);
					}
				}

				propagatedKeyEvent.SetState(keys[keyCode].GetState());
			}

			if(currentState.IsPressed() && keyEvent.IsReleased()) {
				keys[keyCode].RemoveStates(KeyState::PRESSED);
				keys[keyCode].MergeStates(KeyState::EDGE);
				propagatedKeyEvent.SetState(keys[keyCode].GetState());
			}

			if(keyEvent.IsPressed() && currentState.IsPressed()) {
				propagatedKeyEvent.SetState(currentState.GetState() | KeyState::PULSE);
			}

			UpdateModifiers();

			InputEvent(propagatedKeyEvent);
		}

		void ProcessMouseScrollEvent(int delta) {
			OnScroll.Invoke(delta, modifiers);
		}


		void ProcessMouseMoveEvent(const Int2 & mouseWindowPosition)
		{
			this->mouseWindowPosition = mouseWindowPosition;

			OnMouseMove.Invoke(mouseWindowPosition, modifiers);
		}

		void CompleteFrame() {
			for(uint32_t i = 0; i < ARRAYSIZE(iterableKeyCodes); ++i) {
				keys[iterableKeyCodes[i]].RemoveStates(KeyState::EDGE);
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
				axisMap->Update(keys.GetView());
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

		bool InputIsCapital() {
			return GetKey(KeyCode::CAPS_LOCK).IsToggled() ^ GetKey(KeyCode::SHIFT).IsPressed();
		}

		Int2 GetMousePosition()
		{
			return mouseWindowPosition;
		}

		void SetAxisMap(AxisMapRef axMap) {
			axisMap = std::move(axMap);
		}

		Key GetKey(KeyCode code) {
			UndefinedBehaviourAssertion(static_cast<uint32_t>(code) < 256);

			return keys[code];
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
					IF_PRESENT(RI_MOUSE_LEFT_BUTTON_DOWN) ProcessKeyEvent(Key{ KeyCode::MOUSE_LEFT, KeyState::PRESSED });
					IF_PRESENT(RI_MOUSE_LEFT_BUTTON_UP) ProcessKeyEvent(Key{ KeyCode::MOUSE_LEFT, KeyState::RELEASED });

					IF_PRESENT(RI_MOUSE_RIGHT_BUTTON_DOWN) ProcessKeyEvent(Key{ KeyCode::MOUSE_RIGHT, KeyState::PRESSED });
					IF_PRESENT(RI_MOUSE_RIGHT_BUTTON_UP) ProcessKeyEvent(Key{ KeyCode::MOUSE_RIGHT, KeyState::RELEASED });

					IF_PRESENT(RI_MOUSE_MIDDLE_BUTTON_DOWN) ProcessKeyEvent(Key{ KeyCode::MOUSE_MIDDLE, KeyState::PRESSED });
					IF_PRESENT(RI_MOUSE_MIDDLE_BUTTON_UP) ProcessKeyEvent(Key{ KeyCode::MOUSE_MIDDLE, KeyState::RELEASED });

					IF_PRESENT(RI_MOUSE_BUTTON_4_DOWN) ProcessKeyEvent(Key{ KeyCode::MOUSE4, KeyState::PRESSED });
					IF_PRESENT(RI_MOUSE_BUTTON_4_UP) ProcessKeyEvent(Key{ KeyCode::MOUSE4, KeyState::RELEASED });

					IF_PRESENT(RI_MOUSE_BUTTON_5_DOWN) ProcessKeyEvent(Key{ KeyCode::MOUSE5, KeyState::PRESSED });
					IF_PRESENT(RI_MOUSE_BUTTON_5_UP) ProcessKeyEvent(Key{ KeyCode::MOUSE5, KeyState::RELEASED });

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
	Input::EventType<wchar_t> * Input::OnCharInput{ nullptr };

	bool Input::InputIsCapital() {
		return instance->InputIsCapital();
	}

	bool Input::IsAlphabetic(KeyCode keyCode) {
		return keyCode >= KeyCode::A && keyCode <= KeyCode::Z;
	}

	bool Input::IsNumber(KeyCode keyCode) {
		return (keyCode >= KeyCode::_0 && keyCode <= KeyCode::_9);
	}

	bool Input::IsAlphaNumeric(KeyCode keyCode) {
		return IsNumber(keyCode) || IsAlphabetic(keyCode);
	}

	bool Input::IsModifier(KeyCode keyCode) {
		return MapKeyModifier(keyCode) != KeyModifier::NONE;
	}

	bool Input::IsToggledType(KeyCode keyCode) {
		return keyCode == KeyCode::CAPS_LOCK || keyCode == KeyCode::SCROLL_LOCK || keyCode == KeyCode::NUM_LOCK;
	}

	Key Input::GetKey(KeyCode keyCode) {
		return instance->GetKey(keyCode);
	}

	void Input::ProcessBlurredEvent() {
		instance->ProcessBlurredEvent();
	}

	void Input::ProcessFocusedEvent() {
		instance->ProcessFocusedEvent();
	}

	void Input::ProcessKeyEvent(Key keyEvent) {
		instance->ProcessKeyEvent(keyEvent);
	}

	void Input::ProcessCharEvent(wchar_t value) {
		instance->ProcessCharEvent(value);
	}

	void Input::ProcessMouseMoveEvent(const Int2 & mouseWindowPosition) {
		instance->ProcessMouseMoveEvent(mouseWindowPosition);
	}

	void Input::ProcessPlatformEvent(uintptr_t wParam, intptr_t lParam) {
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
			OnCharInput = &instance->OnCharInput;

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

	void Input::CompleteFrame() {
		instance->CompleteFrame();
	}

	Int2 Input::GetMouseDelta() {
		return instance->GetMouseDelta();
	}

	Int2 Input::GetMousePosition() {
		return instance->GetMousePosition();
	}

	KeyModifier operator&(KeyModifier lhs, KeyModifier rhs)
	{
		return static_cast<KeyModifier>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
	}

	KeyModifier operator|(KeyModifier lhs, KeyModifier rhs)
	{
		return static_cast<KeyModifier>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	bool KeyModifierContains(KeyModifier lhs, KeyModifier rhs)
	{
		// if rhs is 0, then this call always returns true, which can cause issues
		UndefinedBehaviourAssertion(static_cast<uint32_t>(rhs) != 0);
		return (lhs & rhs) == rhs;
	}

	KeyState operator&(KeyState lhs, KeyState rhs) {
		return static_cast<KeyState>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
	}

	KeyState operator|(KeyState lhs, KeyState rhs) {
		return static_cast<KeyState>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	bool KeyStateContains(KeyState lhs, KeyState rhs) {
		// if rhs is 0, then this call always returns true, which can cause issues
		UndefinedBehaviourAssertion(static_cast<uint32_t>(rhs) != 0);
		return (lhs & rhs) == rhs;
	}

}
