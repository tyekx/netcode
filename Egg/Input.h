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

		static DirectX::XMINT2 mousePos;
		static DirectX::XMINT2 mouseDelta;
		static bool isFocused;
		static std::map<std::string, Axis> axisMap;
		static unsigned char inputBuffer[2048];
	public:
		static void CreateResources();
		static void ReadRawMouse(unsigned long long wParam, unsigned long long lParam);
		static void SetMousePos(const DirectX::XMINT2 & pos);

		static void KeyPressed(uint32_t keyCode);
		static void KeyReleased(uint32_t keyCode);

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
