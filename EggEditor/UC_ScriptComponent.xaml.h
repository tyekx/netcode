//
// UC_ScriptComponent.xaml.h
// Declaration of the UC_ScriptComponent class
//

#pragma once

#include "UC_ScriptComponent.g.h"

namespace EggEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_ScriptComponent sealed
	{
	public:
		UC_ScriptComponent();
	private:
		void TextBox_DragOver(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e);
		void TextBox_Drop(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e);
	};
}
