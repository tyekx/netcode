#include "TextBox.h"
#include <Netcode/Utility.h>
#include <Netcode/Functions.h>
#include <Netcode/Graphics/SpriteDesc.h>
#include <physx/PxRigidDynamic.h>
#include <physx/PxScene.h>

namespace Netcode::UI {

	static wchar_t WIDE_STARS[] = {
		L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',
		L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*',  L'*', };

	Weak<Panel> TextBox::globalCaret{};

	Ref<Panel> TextBox::CreateCaret(const Memory::ObjectAllocator & alloc)
	{
		Ref<Panel> panel = std::make_shared<Panel>(alloc, nullptr);

		panel->Size(Netcode::Float2{ 1.0f, 26.0f });
		panel->BackgroundColor(Netcode::Float4::One);
		panel->AddAnimation(MakeAnimation(
			&Panel::Opacity,
			&Panel::Opacity,
			Interpolator<float>{ 0.0f, 1.0f },
			RepeatBehaviour{},
			&Function::InvHalfStep, 1.0f));

		return panel;
	}

	std::wstring_view TextBox::GetView() const
	{
		std::wstring_view view;
		static std::wstring overflowedPasswordFieldContent;

		if(IsPassword()) {
			constexpr size_t staticArraySize = Utility::ArraySize(WIDE_STARS);
			const size_t tSize = Text().size();

			if(tSize > staticArraySize) {
				if(tSize > overflowedPasswordFieldContent.size()) {
					overflowedPasswordFieldContent = std::wstring(tSize, L'*');
				}
				view = overflowedPasswordFieldContent;
			} else {
				view = std::wstring_view{ WIDE_STARS, tSize };
			}
		} else {
			view = Text();
		}

		return view;
	}

	int32_t TextBox::GetTargetedCaretPosition(float xWindowPos)
	{
		if(font != nullptr) {
			float localX = xWindowPos - ScreenPosition().x + xSlider - Padding().x;

			if(localX < 0.0f && xSlider == 0.0f) {
				return 0;
			}

			float x = 0.0f;
			std::wstring_view view = GetView();
			size_t i = 0;
			for(const wchar_t * it = view.data(); i < view.size(); ++it, ++i) {
				const Glyph * g = font->FindGlyph(*it);

				float xOff = g->XOffset;
				if(i == 0) {
					xOff = 0.0f;
				}

				float glyphWidth = static_cast<float>(g->Subrect.right - g->Subrect.left) + g->XAdvance + xOff;
				float xNom = x + glyphWidth;

				if(x < localX && xNom >= localX) {
					float xPos = localX - x;
					xPos -= glyphWidth;

					if(xPos < 0.0f) {
						return i;
					} else {
						return i + 1;
					}
				}

				x = xNom;
			}

			return i;
		}

		return 0;
	}

	void TextBox::PropagateOnMouseEnter(MouseEventArgs & evtArgs) {
		evtArgs.Handled(true);
		Input::PropagateOnMouseEnter(evtArgs);
	}

	void TextBox::PropagateOnMouseLeave(MouseEventArgs & evtArgs) {
		evtArgs.Handled(true);
		Input::PropagateOnMouseLeave(evtArgs);
	}

	void TextBox::PropagateOnClick(MouseEventArgs & evtArgs) {
		int32_t targetedIndex = GetTargetedCaretPosition(evtArgs.Position().x);

		if(evtArgs.Modifier() == KeyModifier::SHIFT) {
			selectionOffset = targetedIndex - caretPosition;
		} else {
			caretPosition = targetedIndex;
			selectionOffset = 0;
		}

		caretRef->Animations().RestartAnimation(0);

		evtArgs.Handled(true);
		Input::PropagateOnClick(evtArgs);
	}

	void TextBox::PropagateOnKeyPressed(KeyEventArgs & args)
	{
		args.Handled(true);
		Input::PropagateOnKeyPressed(args);
	}

	void TextBox::PropagateOnCharInput(CharInputEventArgs & args)
	{
		args.Handled(true);
		Input::PropagateOnCharInput(args);
	}

	void TextBox::PropagateOnBlurred(FocusChangedEventArgs & args)
	{
		caretPosition = Text().size();
		selectionOffset = 0;

		Input::PropagateOnBlurred(args);
	}

	void TextBox::PropagateOnMouseKeyPressed(MouseEventArgs & args) {
		args.Handled(true);

		if(args.Key().GetCode() == KeyCode::MOUSE_LEFT) {
			MouseEventArgs copyArgs{ args.Position(), args.Key(), args.Modifier() };
			copyArgs.Handled(true);
			copyArgs.HandledBy(this);
			PropagateOnClick(copyArgs);
		}

		Input::PropagateOnMouseKeyPressed(args);
	}

	void TextBox::PropagateOnMouseKeyReleased(MouseEventArgs & args) {
		args.Handled(true);
		Input::PropagateOnMouseKeyReleased(args);
	}

	void TextBox::PropagateOnDrag(DragEventArgs & args) {
		args.Handled(true);

		int32_t targetedIndex = GetTargetedCaretPosition(args.Position().x);
		selectionOffset = targetedIndex - caretPosition;

		Input::PropagateOnDrag(args);
	}

	void TextBox::PropagateOnTextChanged()
	{
		Input::PropagateOnTextChanged();
		OnTextChanged.Invoke(Text());
	}

	void TextBox::Render(Ptr<SpriteBatch> batch) {
		Panel::Render(batch);

		if(font == nullptr) {
			return;
		}

		std::wstring_view view = GetView();
		const Float4 paddingF4 = Padding();
		const Vector2 paddingLeftTop = Netcode::Float2{ paddingF4.x, paddingF4.y };
		const Vector2 paddingRightBottom = Netcode::Float2{ paddingF4.z, paddingF4.w };
		const Vector2 screenPos = ScreenPosition();
		const Vector2 textPos = TextPosition();
		const Vector2 unitX = Float2::UnitX;
		int32_t offsettedCaretPosition = caretPosition + selectionOffset;
		float heightestCharHeight = font->GetHighestAlphaNumericCharHeight();
		float caretSelectionWidth = font->MeasureString(std::wstring_view{ view.data(), static_cast<size_t>(offsettedCaretPosition) }).x;
		const Vector2 ctrlSize = Size();
		const Float2 textAreaSize = ctrlSize - paddingLeftTop - paddingRightBottom;
		const Float2 textSize = font->MeasureString(view);
		float caretHeight = heightestCharHeight + 4.0f;

		if(textSize.x < (textAreaSize.x + 0.5f)) {
			xSlider = 0.0f;
		} else {
			if(caretSelectionWidth < xSlider) {
				if(offsettedCaretPosition <= 1) {
					xSlider = 0.0f;
				} else {
					float caretMin1Width = font->MeasureString(std::wstring_view{ view.data(), static_cast<size_t>(offsettedCaretPosition - 1) }).x;
					xSlider = caretMin1Width;
				}
			} else if((xSlider + textAreaSize.x) < caretSelectionWidth) {
				if(static_cast<size_t>(offsettedCaretPosition) >= (Text().size() - 1)) {
					xSlider = caretSelectionWidth - textAreaSize.x;
				} else {
					float caretPlus1Width = font->MeasureString(std::wstring_view{ view.data(), static_cast<size_t>(offsettedCaretPosition + 1) }).x;
					xSlider = caretPlus1Width - textAreaSize.x;
				}
			}
		}

		if(!text.empty()) {
			const Float2 textAreaLeftTopCorner = screenPos + paddingLeftTop;

			const Vector2 negSlider = Float2{ -xSlider, 0.0f };

			Netcode::Rect scissorRect;
			scissorRect.left = static_cast<int32_t>(textAreaLeftTopCorner.x);
			scissorRect.top = static_cast<int32_t>(textAreaLeftTopCorner.y);
			scissorRect.right = scissorRect.left + static_cast<int32_t>(textAreaSize.x);
			scissorRect.bottom = scissorRect.top + static_cast<int32_t>(textAreaSize.y);

			batch->SetScissorRect(scissorRect);

			const Vector2 basePosition = screenPos + paddingLeftTop + negSlider;
			const Vector2 baseTextPosition =  basePosition + textPos;
			
			if(selectionOffset != 0 && Focused()) {
				int32_t minValue = std::min(caretPosition, offsettedCaretPosition);
				int32_t maxValue = std::max(caretPosition, offsettedCaretPosition);
				int32_t absValue = std::abs(selectionOffset);

				std::wstring_view unselectedLeft{ view.data(), static_cast<size_t>(minValue) };
				std::wstring_view selectedMiddle{ view.data() + minValue, static_cast<size_t>(absValue) };
				std::wstring_view unselectedRight{ view.data() + maxValue, view.size() - static_cast<size_t>(maxValue) };

				Vector2 offsetLeft = Float2::Zero;

				const Glyph* middleFirstGlyph = font->FindGlyph(selectedMiddle.front());
				const Glyph* middleLastGlyph = font->FindGlyph(selectedMiddle.back());

				float leftCorrection = 0.0f;
				float rightCorrection = 0.0f;

				if(!unselectedLeft.empty()) {
					font->DrawString(batch, unselectedLeft, baseTextPosition, TextColor(), RotationOrigin(), RotationZ(), ZIndex() + 0.1f);

					offsetLeft += font->MeasureString(unselectedLeft);
					const Glyph * lastGlyph = font->FindGlyph(unselectedLeft.back());
					leftCorrection = lastGlyph->XAdvance + middleFirstGlyph->XOffset;
				}

				if(!unselectedRight.empty()) {
					const Glyph * firstChar = font->FindGlyph(unselectedRight.front());
					rightCorrection = middleLastGlyph->XAdvance + firstChar->XOffset;
				}

				Float2 selectedTextSize = font->MeasureString(selectedMiddle);
				selectedTextSize.y = caretHeight;
				Float2 selectionAnchorOffset = CalculateAnchorOffset(HorizontalContentAlignment(), VerticalAnchor::MIDDLE, selectedTextSize);
				Float2 selectionRectPosition = paddingLeftTop + basePosition + selectionAnchorOffset + unitX * offsetLeft + Float2{ 0.0f, -BorderWidth() };

				offsetLeft += Float2{ leftCorrection, 0.0f };

				Float2 renderedSize = Float2{ selectedTextSize.x + leftCorrection + rightCorrection, caretHeight };

				batch->DrawSprite(SpriteDesc{ selectionBackgroundColor }, selectionRectPosition, renderedSize);

				font->DrawString(batch, selectedMiddle, baseTextPosition + unitX * offsetLeft, selectionTextColor, RotationOrigin(), RotationZ(), ZIndex() + 0.1f);
				offsetLeft += selectedTextSize;

				if(!unselectedRight.empty()) {
					offsetLeft += Float2{ rightCorrection, 0.0f };

					font->DrawString(batch, unselectedRight, baseTextPosition + unitX * offsetLeft, TextColor(), RotationOrigin(), RotationZ(), ZIndex() + 0.1f);
				}

			} else {

				font->DrawString(batch, view, baseTextPosition, TextColor(), RotationOrigin(), RotationZ(), ZIndex() + 0.1f);

			}

			batch->SetScissorRect();
		}

		if(Focused()) {
			caretRef->Size(Netcode::Float2{ 1.0f, caretHeight });
			Float4 bgColor = Float4{ caretColor.x, caretColor.y, caretColor.z, caretColor.w * caretRef->Opacity() };
			Netcode::Vector2 caretAnchorOffset = CalculateAnchorOffset(HorizontalAnchor::CENTER, VerticalAnchor::MIDDLE, caretRef->Size());
			caretRef->Position(screenPos + paddingLeftTop + unitX * textPos + Float2{ caretSelectionWidth - xSlider, -BorderWidth() } + caretAnchorOffset);
			caretRef->BackgroundColor(bgColor);
			caretRef->Render(batch);
		}
	}

	void TextBox::EraseSelection()
	{
		int32_t cp = caretPosition;
		int32_t cpso = caretPosition + selectionOffset;

		text.erase(std::min(cp, cpso), std::max(cp, cpso) - std::min(cp, cpso));

		caretPosition = std::min(cp, cpso);
		selectionOffset = 0;

		OnTextChanged.Invoke(Text());
	}

	void TextBox::AppendChar(wchar_t c)
	{
		if(selectionOffset != 0) {
			EraseSelection();
		}
		text.insert(text.begin() + caretPosition, c);
		UpdateTextPosition();
		caretPosition += 1;
		OnTextChanged.Invoke(Text());
	}

	void TextBox::Update(float dt) {
		Input::Update(dt);

		if(Focused()) {
			caretRef->Update(dt);
		}
	}

	void TextBox::HandleDefaultKeyStrokes(Key key, KeyModifiers modifier)
	{
		if(key.GetCode() == KeyCode::BACKSPACE) {
			if(selectionOffset != 0) {
				EraseSelection();
				caretRef->Animations().RestartAnimation(0);
			} else {
				if(caretPosition > 0) {
					selectionOffset = -1;
					EraseSelection();
					caretRef->Animations().RestartAnimation(0);
				}
			}
		}

		if(key.GetCode() == KeyCode::A && modifier.IsSet(KeyModifier::CTRL)) {
			caretPosition = 0;
			selectionOffset = static_cast<int32_t>(text.size());
		}

		if(modifier.IsSet(KeyModifier::SHIFT)) {
			if(key.GetCode() == KeyCode::LEFT) {
				if((caretPosition + (selectionOffset - 1)) >= 0) {
					selectionOffset -= 1;
					caretRef->Animations().RestartAnimation(0);
				}
			}

			if(key.GetCode() == KeyCode::RIGHT) {
				if((caretPosition + (selectionOffset + 1)) <= static_cast<int32_t>(text.size())) {
					selectionOffset += 1;
					caretRef->Animations().RestartAnimation(0);
				}
			}

			if(key.GetCode() == KeyCode::HOME) {
				selectionOffset = -caretPosition;
			}

			if(key.GetCode() == KeyCode::END) {
				selectionOffset = static_cast<int32_t>(text.size()) - caretPosition;
			}
		} else {
			if(key.GetCode() == KeyCode::LEFT) {
				if(selectionOffset != 0) {
					if(selectionOffset < 0) {
						caretPosition += selectionOffset;
					}
					selectionOffset = 0;
					caretRef->Animations().RestartAnimation(0);
				} else if(caretPosition > 0) {
					caretPosition -= 1;
					caretRef->Animations().RestartAnimation(0);
				}
			}

			if(key.GetCode() == KeyCode::RIGHT) {
				if(selectionOffset != 0) {
					if(selectionOffset > 0) {
						caretPosition += selectionOffset;
					}
					selectionOffset = 0;
					caretRef->Animations().RestartAnimation(0);
				} else if(caretPosition < static_cast<int32_t>(text.size())) {
					caretPosition += 1;
					caretRef->Animations().RestartAnimation(0);
				}
			}

			if(key.GetCode() == KeyCode::HOME) {
				caretPosition = 0;
				selectionOffset = 0;
			}

			if(key.GetCode() == KeyCode::END) {
				caretPosition = static_cast<int32_t>(text.size());
				selectionOffset = 0;
			}
		}
	}

	TextBox::TextBox(const AllocType & allocator, PxPtr<physx::PxRigidDynamic> pxActor) : Input{ allocator, std::move(pxActor) },
		selectionTextColor{ Float4::One },
		selectionBackgroundColor{ Float4::One },
		caretColor{ Float4::UnitW },
		xSlider{ 0.0f }, caretPosition{ 0 }, selectionOffset{ 0 }, isPassword{ false }, caretRef{}, OnTextChanged{ allocator } {
		if(Utility::IsWeakRefEmpty(globalCaret)) {
			caretRef = CreateCaret(allocator);
			globalCaret = caretRef;
		} else {
			caretRef = globalCaret.lock();
		}
	}

	void TextBox::SelectionTextColor(const Float4 & color)
	{
		selectionTextColor = color;
	}

	Float4 TextBox::SelectionTextColor() const
	{
		return selectionTextColor;
	}

	void TextBox::SelectionBackgroundColor(const Float4 & color)
	{
		selectionBackgroundColor = color;
	}

	Float4 TextBox::SelectionBackgroundColor() const
	{
		return selectionBackgroundColor;
	}

	void TextBox::CaretColor(const Float4 & color)
	{
		caretColor = color;
	}

	Float4 TextBox::CaretColor() const
	{
		return caretColor;
	}

	bool TextBox::IsPassword() const {
		return isPassword;
	}

	void TextBox::IsPassword(bool isPw) {
		isPassword = isPw;
	}

}
