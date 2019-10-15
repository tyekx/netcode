#include "Input.h"

namespace Egg {

	bool Input::IsFocused{ false };
	DirectX::XMINT2 Input::LastMousePos{ -1, -1 };
	DirectX::XMINT2 Input::MouseDelta{};
	std::map<std::string, Input::Axis> Input::AxisMap{ };

	Input::Axis::Axis() : PositiveKey{ 0 }, NegativeKey{ 0 }, CurrentValue{ 0.0f } { }

	Input::Axis::Axis(uint32_t posK, uint32_t negK) : PositiveKey{ posK }, NegativeKey{ negK }, CurrentValue{ 0.0f } { }

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
		if(!IsFocused) {
			return;
		}
		if(LastMousePos.x == -1 && LastMousePos.y == -1) {
			LastMousePos = xy;
			return;
		}

		MouseDelta.x += xy.x - LastMousePos.x;
		MouseDelta.y += xy.y - LastMousePos.y;

		LastMousePos = xy;
	}

	DirectX::XMINT2 Input::GetMouseDelta() {
		return MouseDelta;
	}

	void Input::Reset() {
		MouseDelta = DirectX::XMINT2{ 0,0 };
	}

}
