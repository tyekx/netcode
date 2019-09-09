//
// UC_MeshComponent.xaml.h
// Declaration of the UC_MeshComponent class
//

#pragma once

#include "UC_MeshComponent.g.h"

namespace EggEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_MeshComponent sealed
	{
	public:
		UC_MeshComponent();
	private:
		void ScriptRefTextBox_DragOver(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e);
		void ScriptRefTextBox_Drop(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e);
	};
}
