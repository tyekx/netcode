//
// UC_GameObject.xaml.h
// Declaration of the UC_GameObject class
//

#pragma once

#include "UC_GameObject.g.h"
#include "Auxiliary.h"
#include <Egg/GameObject.h>

namespace EggEditor
{

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_GameObject sealed
	{
		Egg::GameObject * goRef;
		GameObjectDataContext ^ TypedDataContext;
	public:
		UC_GameObject(UINT_PTR gameObjectPtr);

		UINT_PTR GetGameObject() {
			return reinterpret_cast<UINT_PTR>(goRef);
		}


		void Grid_DragOver(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e);
		void Grid_Drop(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e);
		void Grid_DragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args);
	private:
		void CtxRenameBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void TextBox_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e);
	};
}
