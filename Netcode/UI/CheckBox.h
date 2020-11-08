#pragma once

#include "Input.h"

namespace Netcode::UI {

	class CheckBox : public Input {
		Ref<ResourceViews> checkedImage;
		UInt2 checkedImageSize;

		virtual void Render(Ptr<SpriteBatch> batch) override;
	public:
		using Input::Input;

		void CheckedImage(Ref<ResourceViews> view, const UInt2 & imgSize) { checkedImage = std::move(view); checkedImageSize = imgSize; }
	};
	
}
