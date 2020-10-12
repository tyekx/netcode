#pragma once

#include "StackPanel.h"
#include "TextBox.h"

namespace Netcode::UI {

	class Slider : public StackPanel {
	protected:
		Ref<Panel> sliderPanel;
		Ref<Panel> sliderButton;
		Ref<TextBox> sliderValue;
		double minValue;
		double maxValue;
		double value;
		EventToken onTextChangedToken;
		bool textUpdating;

		void UpdateLabelText();
		
	public:
		EventType<double> OnValueChanged;

		void UpdateFromSlider(const Int2& deltaPosition);

		void UpdateFromText(const std::wstring & value);

		void UpdateFromValue(double d);
		
		Slider(Memory::ObjectAllocator controlAllocator, const AllocType & alloc, PxPtr<physx::PxRigidDynamic> actor);

		double MinValue() const { return minValue; }
		void MinValue(double mv) { minValue = mv; }

		double MaxValue() const { return maxValue; }
		void MaxValue(double mv) { maxValue = mv; }

		double Value() const { return value; }
		void Value(double v) { value = v; }

		void PropagateOnDrag(DragEventArgs & args) override;

		void BindTextBox(Ref<TextBox> textBox);

		Ref<Panel> SliderBar() const {
			return sliderPanel;
		}

		Ref<Panel> SliderButton() const {
			return sliderButton;
		}

		virtual void AddChild(Ref<Control> child) override;

		virtual void UpdateLayout() override;
	};
	
}
