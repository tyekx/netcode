#pragma once

#include <NetcodeFoundation/Memory.h>
#include <NetcodeFoundation/Math.h>

#include <algorithm>
#include <string>
#include <map>
#include "Utility.h"
#include "Event.hpp"


#include <functional>

namespace Netcode {

	enum KeyModifier {
		NONE = 0,
		CTRL = 1, ALT = 2, SHIFT = 4,
		CTRL_ALT = ( CTRL | ALT ),
		SHIFT_ALT = ( SHIFT | ALT )
	};

	/*
	LSB   = "Is active?"
	LSB+1 = "Is edge?"
	*/
	enum class KeyState : uint32_t {
		INACTIVE = 0,
		ACTIVE = 1,
		FALLING_EDGE = 2,
		RISING_EDGE = 3,
		PULSE = 5,
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
		SCROLL_LOCK = 0x91
	};

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

		inline KeyState GetState() const {
			return state;
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
			return static_cast<uint32_t>(GetState()) & (1U);
		}

		bool IsReleased() const {
			return !IsPressed();
		}

		bool IsRising() const {
			return GetState() == KeyState::RISING_EDGE;
		}

		bool IsFalling() const {
			return GetState() == KeyState::FALLING_EDGE;
		}

		bool IsEdge() const {
			return static_cast<uint32_t>(GetState()) & (2U);
		}

		bool operator==(KeyCode keyCode) const {
			return code == keyCode;
		}
	};

	enum DefaultAxes {
		VERTICAL,
		HORIZONTAL,
		FIRE1,
		FIRE2,
		JUMP
	};

	class AxisMapBase {
	public:
		virtual ~AxisMapBase() = default;
		virtual float GetAxis(uint32_t axisId) = 0;
		virtual void Update(ArrayView<Key> keys) = 0;
	};

	using AxisMapRef = std::shared_ptr<AxisMapBase>;

	template<typename AxisEnum = DefaultAxes>
	class AxisData {
	public:
		AxisEnum axisId;
		KeyCode positiveKey;
		KeyCode negativeKey;
		float value;

		AxisData() : axisId{ 0 }, positiveKey{ KeyCode::UNDEFINED }, negativeKey{ KeyCode::UNDEFINED }, value{ 0.0f } { }
		AxisData(AxisEnum axisId, KeyCode posKey, KeyCode negKey) :
			axisId{ axisId }, positiveKey{ posKey }, negativeKey{ negKey }, value{ 0.0f } { }
	};

	template<typename AxisEnum = DefaultAxes>
	class AxisMap : public AxisMapBase {

		using ContainerType = std::vector<AxisData<AxisEnum>>;

		ContainerType data;
	public:

		AxisMap(const std::initializer_list<typename ContainerType::value_type> & initList) : data{} {
			uint32_t maxId = 0;

			for(const auto & i : initList) {
				uint32_t axisId = static_cast<uint32_t>(i.axisId);
				maxId = std::max(axisId, maxId);
			}

			ASSERT(maxId <= 64, "AxisMap enum value is out of range. Valid range: [0, 64]");

			data.resize(maxId + 1);

			for(const auto & i : initList) {
				data[static_cast<uint32_t>(i.axisId)] = i;
			}
		}

		virtual float GetAxis(uint32_t axisId) override {
			ASSERT(axisId < static_cast<uint32_t>(data.size()), "Out of range");

			return data[axisId].value;
		}

		virtual void Update(ArrayView<Key> keys) override {
			for(AxisData<AxisEnum> & axisData : data) {
				Key posKey = keys[static_cast<uint32_t>(axisData.positiveKey)];
				Key negKey = keys[static_cast<uint32_t>(axisData.negativeKey)];

				if(posKey.IsPressed() && negKey.IsReleased()) {
					axisData.value = 1.0f;
				}

				if(posKey.IsReleased() && negKey.IsReleased()) {
					axisData.value = -1.0f;
				}

				// both is in the same state
				if(posKey.IsPressed() == negKey.IsPressed()) {
					axisData.value = 0.0f;
				}
			}
		}
	};

	class Input {
		Input() = delete;
		~Input() = delete;

		struct detail;

		static std::unique_ptr<detail> instance;
	public:

		using Alloc = Memory::ObjectAllocator;

		using StdAlloc = Memory::StdAllocatorAdapter<void, Alloc>;

		template<typename ... T>
		using EventType = ManagedEvent<StdAlloc, T...>;

		/**
		* Raw input, every key, every mouse button, every state
		*/
		static EventType<Key, KeyModifier> * OnInput;

		/**
		* Raw mouse input, every mouse key, every state
		*/
		static EventType<Key, KeyModifier> * OnMouseInput;

		/**
		* Rising edges for mouse keys
		*/
		static EventType<Key, KeyModifier> * OnMouseKeyPressed;

		/**
		* Falling edges for mouse keys
		*/
		static EventType<Key, KeyModifier> * OnMouseKeyReleased;

		/**
		* Raw keyboard input, every keyboard key, every state
		*/
		static EventType<Key, KeyModifier> * OnKeyInput;

		/**
		* Rising edges or pulses for keyboard keys
		*/
		static EventType<Key, KeyModifier> * OnKeyPressed;

		/**
		* Falling edges for keyboard keys
		*/
		static EventType<Key, KeyModifier> * OnKeyReleased;

		/**
		* Mouse wheel scroll event
		* 1st argument is a 1D vector, direction and longitude. 
		* x > 0: scroll up
		* x < 0: scroll down
		*/
		static EventType<int, KeyModifier> * OnScroll;

		/**
		* Mouse move event
		* Int2: mouse delta
		*/
		static EventType<Int2, KeyModifier> * OnMouseMove;


		

		static void ProcessEvent(Key keyEvent);

		static void ProcessMouseMoveEvent(const Int2 & mouseWindowPosition);

		static void ProcessPlatformEvent(uintptr_t wParam, intptr_t lParam);

		static void Initialize();

		static void UpdateAxisMap(float dt);

		static float GetAxis(uint32_t axis);

		static void SetAxisMap(AxisMapRef axisMap);

		static void CompleteFrame();

		static Int2 GetMouseDelta();

		static Int2 GetMousePosition();

	};

}
