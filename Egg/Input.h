#pragma once

#include <string>
#include <map>
#include "Utility.h"

#include <DirectXMath.h>

namespace Egg {

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

		static DirectX::XMINT2 LastMousePos;
		static DirectX::XMINT2 MouseDelta;
		static bool IsFocused;
		static std::map<std::string, Axis> AxisMap;
		static unsigned char InputBuffer[2048];
	public:
		static void CreateResources();
		static void ReadRawMouse(unsigned long long wParam, unsigned long long lParam);

		static void KeyPressed(uint32_t keyCode);
		static void KeyReleased(uint32_t keyCode);

		static float GetAxis(const std::string & axis);
		static void SetAxis(const std::string & name, uint32_t posKey, uint32_t negKey);

		static void MouseMove(const DirectX::XMINT2 & xy);
		static DirectX::XMINT2 GetMouseDelta();

		static void Blur();

		static void Focused();

		/*
		* Reset must be called after updating the scene and processing the inputs, otherwise some inputs will not register
		*/
		static void Reset();

	};
}
