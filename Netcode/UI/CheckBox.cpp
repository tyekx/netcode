#include "CheckBox.h"
#include <Netcode/Graphics/SpriteDesc.h>

namespace Netcode::UI {

	void CheckBox::Render(Ptr<SpriteBatch> batch) {
		Input::Render(batch);
		batch->DrawSprite(SpriteDesc{ checkedImage.get(), checkedImageSize }, Vector2{ ScreenPosition() } - Float2{ 0.0f, 12.0f });
	}
	
}
