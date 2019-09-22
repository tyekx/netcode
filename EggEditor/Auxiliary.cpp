#include "pch.h"
#include "Auxiliary.h"
#include <string>
#include "UC_GameObject.xaml.h"

Platform::Object ^ EggEditor::IntEqualConverter::Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language)
{
	Platform::String ^ v = (Platform::String ^)parameter;
	if((int32)value == std::stoi(v->Data())) {
		return Windows::UI::Xaml::Visibility::Visible;
	}
	return Windows::UI::Xaml::Visibility::Collapsed;
}

Platform::Object ^ EggEditor::IntEqualConverter::ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language)
{
	throw ref new Platform::NotImplementedException();
}

Platform::Object ^ EggEditor::BoolToVisibilityConverter::Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language)
{
	Platform::String ^ v = (Platform::String ^)parameter;

	if((bool)value == (std::stoi(v->Data()) > 0)) {
		return Windows::UI::Xaml::Visibility::Visible;
	}
	return Windows::UI::Xaml::Visibility::Collapsed;
}

Platform::Object ^ EggEditor::BoolToVisibilityConverter::ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language)
{
	throw ref new Platform::NotImplementedException();
}

Platform::Object ^ EggEditor::IntNotEqualConverter::Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language)
{
	Platform::String ^ v = (Platform::String ^)parameter;
	return (int32)value != std::stoi(v->Data());
}

Platform::Object ^ EggEditor::IntNotEqualConverter::ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language)
{
	throw ref new Platform::NotImplementedException();
}


Platform::Object ^ EggEditor::SignatureToVisibilityConverter::Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language) {
	Platform::String ^ v = (Platform::String ^)parameter;
	uint64 signatureShift = (uint64)std::stoi(v->Data());

	uint64 signature = (uint64)value;
	uint64 signatureMask = 0b1ULL << signatureShift;

	if((signature & signatureMask) > 0) {
		return Windows::UI::Xaml::Visibility::Visible;
	}
	return Windows::UI::Xaml::Visibility::Collapsed;
}

Platform::Object ^ EggEditor::NegatedSignatureToVisibilityConverter::Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language) {
	Platform::String ^ v = (Platform::String ^)parameter;
	uint64 signatureShift = (uint64)std::stoi(v->Data());

	uint64 signature = (uint64)value;
	uint64 signatureMask = 0b1ULL << signatureShift;

	if((signature & signatureMask) == 0) {
		return Windows::UI::Xaml::Visibility::Visible;
	}
	return Windows::UI::Xaml::Visibility::Collapsed;
}

Platform::Object ^ EggEditor::NegatedSignatureToVisibilityConverter::ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language) {
	throw ref new Platform::NotImplementedException();
}


Platform::Object ^ EggEditor::SignatureBitsToVisibilityConverter::Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language) {
	uint64 signature = (uint64)value;
	
	if((signature == TupleMaxMaskValue<COMPONENTS_T>::value)) {
		return Windows::UI::Xaml::Visibility::Collapsed;
	}
	return Windows::UI::Xaml::Visibility::Visible;
}

Platform::Object ^ EggEditor::SignatureBitsToVisibilityConverter::ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language) {
	throw ref new Platform::NotImplementedException();
}

Platform::Object ^ EggEditor::SignatureToVisibilityConverter::ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language) {
	throw ref new Platform::NotImplementedException();
}

EggEditor::SceneDataContext::SceneDataContext() {
	gameObjects = ref new Platform::Collections::Vector<Windows::UI::Xaml::UIElement ^>();
}
