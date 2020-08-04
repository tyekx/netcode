#pragma once

#include <cstdint>

namespace Netcode {


	enum class KeyModifier : uint32_t {
		NONE = 0,
		CTRL = 1, ALT = 2, SHIFT = 4,
		CTRL_ALT = (CTRL | ALT),
		SHIFT_ALT = (SHIFT | ALT)
	};

	/*
	LSB   = "Is active?"
	LSB+1 = "Is edge?"
	*/
	enum class KeyState : uint32_t {
		UP = 0,
		DOWN = 1,
		RELEASED = (UP),
		PRESSED = (DOWN),
		EDGE = 2,
		FALLING_EDGE = (EDGE | RELEASED),
		RISING_EDGE = (EDGE | PRESSED),
		PULSE = 4,
		TOGGLED = 8,
		UNDEFINED = 0xFFFFFFFF
	};

	enum class KeyCode : uint32_t {
		UNDEFINED = 0x00,

		MOUSE_LEFT = 0x01,
		MOUSE_RIGHT = 0x02,
		MOUSE_MIDDLE = 0x04,
		MOUSE4 = 0x05,
		MOUSE5 = 0x06,

		BACKSPACE = 0x08,
		TAB = 0x09,

		CLEAR = 0x0C,
		RETURN = 0x0D,

		SHIFT = 0x10,
		CONTROL = 0x11,
		ALT = 0x12,
		PAUSE = 0x13,
		CAPS_LOCK = 0x14,

		ESCAPE = 0x18,
		SPACE = 0x20,
		PAGE_UP = 0x21,
		PAGE_DOWN = 0x22,
		END = 0x23,
		HOME = 0x24,
		LEFT = 0x25,
		UP = 0x26,
		RIGHT = 0x27,
		DOWN = 0x28,

		PRINT_SCREEN = 0x2C,
		INSERT = 0x2D,
		DEL = 0x2E,

		_0 = 0x30,
		_1 = 0x31,
		_2 = 0x32,
		_3 = 0x33,
		_4 = 0x34,
		_5 = 0x35,
		_6 = 0x36,
		_7 = 0x37,
		_8 = 0x38,
		_9 = 0x39,

		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4A,
		K = 0x4B,
		L = 0x4C,
		M = 0x4D,
		N = 0x4E,
		O = 0x4F,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		W = 0x57,
		X = 0x58,
		Y = 0x59,
		Z = 0x5A,

		NUM_0 = 0x60,
		NUM_1 = 0x61,
		NUM_2 = 0x62,
		NUM_3 = 0x63,
		NUM_4 = 0x64,
		NUM_5 = 0x65,
		NUM_6 = 0x66,
		NUM_7 = 0x67,
		NUM_8 = 0x68,
		NUM_9 = 0x69,

		MULTIPLY = 0x6A,
		ADD = 0x6B,
		SEPARATOR = 0x6C,
		SUBTRACT = 0x6D,
		DECIMAL = 0x6E,
		DIVIDE = 0x6F,

		F1 = 0x70,
		F2 = 0x71,
		F3 = 0x72,
		F4 = 0x73,
		F5 = 0x74,
		F6 = 0x75,
		F7 = 0x76,
		F8 = 0x77,
		F9 = 0x78,
		F10 = 0x79,
		F11 = 0x7A,
		F12 = 0x7B,

		NUM_LOCK = 0x90,
		SCROLL_LOCK = 0x91,

		SHIFT_LEFT = 0xA0,
		SHIFT_RIGHT = 0xA1,
		CONTROL_LEFT = 0xA2,
		CONTROL_RIGHT = 0xA3,
		ALT_LEFT = 0xA4,
		ALT_RIGHT = 0xA5
	};

	KeyModifier operator&(KeyModifier lhs, KeyModifier rhs);
	KeyModifier operator|(KeyModifier lhs, KeyModifier rhs);
	bool KeyModifierContains(KeyModifier lhs, KeyModifier rhs);

	KeyState operator&(KeyState lhs, KeyState rhs);
	KeyState operator|(KeyState lhs, KeyState rhs);
	bool KeyStateContains(KeyState lhs, KeyState rhs);

	class Key {
		KeyCode code;
		KeyState state;
	public:
		Key() : code{ KeyCode::UNDEFINED }, state{ KeyState::UNDEFINED } { }
		Key(KeyCode code, KeyState state) : code{ code }, state{ state } { }
		Key(KeyCode code) : code{ code }, state{ KeyState::UNDEFINED } { }

		inline KeyCode GetCode() const {
			return code;
		}

		/**
		* KeyState can be any combination of the given states. Use a specific
		* query function instead.
		*/
		inline KeyState GetState() const {
			return state;
		}

		inline void MergeStates(KeyState orState) {
			state = state | orState;
		}

		inline void RemoveStates(KeyState negAndState) {
			state = state & static_cast<KeyState>((~static_cast<uint32_t>(negAndState)));
		}

		inline void SetState(KeyState newState) {
			state = newState;
		}

		bool IsMouse() const {
			return code == KeyCode::MOUSE_LEFT || code == KeyCode::MOUSE_RIGHT || code == KeyCode::MOUSE_MIDDLE || code == KeyCode::MOUSE4 || code == KeyCode::MOUSE5;
		}

		bool IsKeyboard() const {
			return code != KeyCode::UNDEFINED && !IsMouse();
		}

		bool IsPressed() const {
			return KeyStateContains(GetState(), KeyState::PRESSED);
		}

		bool IsReleased() const {
			return !IsPressed();
		}

		bool IsToggled() const {
			return KeyStateContains(GetState(), KeyState::TOGGLED);
		}

		bool IsPulse() const {
			return KeyStateContains(GetState(), KeyState::PULSE);
		}

		bool IsEdge() const {
			return KeyStateContains(GetState(), KeyState::EDGE);
		}

		bool IsActive() const {
			return IsPressed() || IsPulse() || IsToggled();
		}

		bool IsInactive() const {
			return !IsActive();
		}

		bool IsRising() const {
			return IsPressed() && IsEdge();
		}

		bool IsFalling() const {
			return IsReleased() && IsEdge();
		}

		bool operator==(KeyCode keyCode) const {
			return code == keyCode;
		}
	};

}
