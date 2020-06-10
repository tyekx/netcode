#pragma once

#include "GameObject.h"
#include <Netcode/Input.h>

class DebugBehavior : public IBehavior {
	float * refs[4];
	float * curr;
	Netcode::EventToken token;
public:
	DebugBehavior(float * x = nullptr, float * y = nullptr, float * z = nullptr, float * w = nullptr) : refs{}, curr{ nullptr }, token{} {
		refs[0] = x;
		refs[1] = y;
		refs[2] = z;
		refs[3] = w;
	}

	virtual void Setup(GameObject * owner) override {
		token = Netcode::Input::OnKeyPressed.Subscribe([this](uint32_t keyCode, uint32_t modifiers) -> void {
			uint32_t idx = keyCode - static_cast<uint32_t>('0');
			if(idx < 4) {
				curr = refs[idx];
				Log::Debug("[DebugScript] selected value of index: {0}", static_cast<int32_t>(idx));
			}


			if(curr != nullptr) {
				float scale = 1.0f;

				if((modifiers & Netcode::KeyModifiers::CTRL) > 0) {
					scale *= 10.0f;
				} else if((modifiers & Netcode::KeyModifiers::ALT) > 0) {
					scale /= 10.0f;
				}

				if(keyCode == VK_ADD) {
					*curr += scale;
					Log::Debug("[DebugScript] value increased to {0}", std::to_string(*curr));
				}

				if(keyCode == VK_SUBTRACT) {
					*curr -= scale;
					Log::Debug("[DebugScript] value decreased to {0}", std::to_string(*curr));
				}
			}

		});
	}

	virtual void Update(float dt) override {
	}
};

