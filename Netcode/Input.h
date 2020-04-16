#pragma once

#include <string>
#include <map>
#include "Utility.h"
#include "Event.hpp"

#include <DirectXMath.h>
#include <functional>

namespace Netcode {

	enum KeyModifiers {
		CTRL = 1, ALT = 2, SHIFT = 4, CAPS_LOCK = 8, 
		CTRL_ALT = ( CTRL | ALT ),
		SHIFT_ALT = ( SHIFT | ALT ),
		SHIFT_CAPS_LOCK = ( SHIFT | CAPS_LOCK )
	};
	/*
	namespace Input {

		class Events {
			~Events() = delete;
			Events() = delete;
		public:

			static Event<uint32_t, uint32_t> OnKeyPressed;
			static Event<uint32_t, uint32_t> OnMouseMoved;
			static Event<> OnFocused;
			static Event<> OnBlurred;
			// [...]
		};

		class InputBase {
		protected:
			static bool isFocused;

		public:
			static void Focus();

			static void Blur();
		};

		class Axes {

		public:
			float GetAxis(const std::string & axis);
			void SetAxis(const std::string & name, uint32_t posKey, uint32_t negKey);
		};

		class Mouse {
			DirectX::XMINT2 cursorPos;
			DirectX::XMINT2 cursorDelta;
			std::bitset<256> keysHeld;
		public:
			DirectX::XMINT2 GetDelta() const {

			}

			DirectX::XMINT2 GetPosition() const {

			}

			void Consume();
		};



	}*/

	class Input {
		~Input() = delete;
		Input() = delete;

		struct Axis {
			uint32_t PositiveKey;
			uint32_t NegativeKey;
			float CurrentValue;

			Axis();
			Axis(uint32_t posK, uint32_t negK);
		};

		static DirectX::XMINT2 mousePos;
		static DirectX::XMINT2 mouseDelta;
		static bool isFocused;
		static std::map<std::string, Axis> axisMap;
		static bool keysHeld[256];
		static unsigned char inputBuffer[2048];
		static uint32_t activeModifiers;
	public:
		static void CreateResources();
		static void ReadRawMouse(unsigned long long wParam, unsigned long long lParam);
		static void SetMousePos(const DirectX::XMINT2 & pos);

		static void KeyPressed(uint32_t keyCode);
		static void KeyReleased(uint32_t keyCode);

		static Event<uint32_t, uint32_t> OnKeyPressed;

		static float GetAxis(const std::string & axis);
		static void SetAxis(const std::string & name, uint32_t posKey, uint32_t negKey);

		static void MouseMove(const DirectX::XMINT2 & xy);
		static DirectX::XMINT2 GetMouseDelta();

		static DirectX::XMINT2 GetMousePos();

		static void Blur();

		static void Focused();

		/*
		* Reset must be called after updating the scene and processing the inputs, otherwise some inputs will not register
		*/
		static void Reset();

	};
}
